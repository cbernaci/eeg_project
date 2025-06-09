/*
 * visualization.c
 *
 * This file contains functions for visualization using Metal and ImGUI.
 * 
 * It contains functions for:
 * - Hello World 
 *   - Metal basic functionality
 * - Concurrent Sine Wave
 *   - a producer thread fills a ring buffer with sine wave data points
 *     a consumer thread reads and sends it to Metal for visualization
 *     ImGui is used to adjust window size of plot 
 *
 * Author: Catherine Bernaciak PhD
 * Date: April - May 2025
*/

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <Foundation/Foundation.h>
#import <simd/simd.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreFoundation/CoreFoundation.h>
#import <objc/runtime.h>
#import <pthread.h>
#import <stdio.h>
#import <stdlib.h>
#import "eeg_config.h"
#import "ring_buffer.h"
#import "visualization.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_APPLICATION_MAIN

#define WIDTH 800
#define HEIGHT 600

// global Metal objects
// connection to GPU driver
id<MTLDevice> device;             
// a queue of GPU commands - contains multiple
// command buffers each of which typically render a frame
id<MTLCommandQueue> commandQueue;  
id<MTLRenderPipelineState> pipelineState; // info for GPU to draw 
CAMetalLayer *metalLayer;

typedef struct {
   vector_float2 position;
} Vertex;

Vertex vertices[NUM_POINTS];
Vertex reordered_vertices[NUM_POINTS];

// compile a simple shader
NSString* shaderSource = @"\
#include <metal_stdlib>                  \n\
using namespace metal;                   \n\
struct VertexIn{                         \n\
   float2 position [[attribute(0)]];     \n\
};                                       \n\
                                         \n\
vertex float4 vertex_main(VertexIn in [[stage_in]]) { \n\
   return float4(in.position, 0.0, 1.0);  \n\
}                                       \n\
                                         \n\
fragment float4 fragment_main(){         \n\
   return float4(0.0, 1.0, 0.0, 1.0); // green line \n\
}                                        \n\
";

// global def of phase of sine wave, used in updateAllVertices()
// and updateOneVertex
float phase = 0.0f; // phase of sine wave

// global buffer for displayed y-values
float display_buffer[NUM_POINTS] = {0};

// to keep track of which vertex to draw up to
int num_points_filled = 0;

void initMetal(){
   // device is main interface to GPU
   device = MTLCreateSystemDefaultDevice();
   // submission pipeline for jobs to GPU
   commandQueue = [device newCommandQueue];
   // setup Metal layer
   metalLayer = [CAMetalLayer layer];
   metalLayer.device = device;   
   // each pixel is BGRA color layout w 8 bits per channel
   // using unsigned normalized integers
   metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;   
   // CAMetalLayers textures only used for rendering to screen and not
   // not for sampling or reading from later
   metalLayer.framebufferOnly = YES;
   // contentsScale tells Metal how high-res rendering target should be
   // NSScreen is a macOS display, with mainScreen is the screen used by app
   // backingScaleFactor is number of pixels per point (2.0 on retina)
   metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
   // position the Metal layer at the origin of it's parent (0,0) with 
   // size of WIDTHxHEIGHT points. 
   metalLayer.frame = CGRectMake(0, 0, WIDTH, HEIGHT);

   // compile shader code into GPU-usable functions
   NSError *error = nil;
   // take the Metal shader code in the string above, compile it at runtime
   // and make a MTLLibrary object containing the compiled functions
   id<MTLLibrary> library = [device newLibraryWithSource:shaderSource options:nil error:&error];
   if (!library){
      NSLog(@"❌  Shader compile error: %@", error.localizedDescription);
      exit(1);
   }
   id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
   id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];
   if (!vertexFunc || !fragmentFunc){
      NSLog(@"❌ Shader functions missing.");
      exit(1); 
   }
   // creates an empty render pipeline descriptor
   MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
   // these shaders run once per pixel
   pipelineDesc.vertexFunction = vertexFunc;
   pipelineDesc.fragmentFunction = fragmentFunc;
   // this tells the GPUs render pipeline to output it's color data in this format
   pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

   // setup vertex layout for shader input
   MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];
   vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
   vertexDescriptor.attributes[0].offset = 0;
   vertexDescriptor.attributes[0].bufferIndex = 0;
   vertexDescriptor.layouts[0].stride = sizeof(Vertex);
   vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
   pipelineDesc.vertexDescriptor = vertexDescriptor;

   // compile the pipeline state
   pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
   if(!pipelineState){
      NSLog(@"❌ Failed to create pipelineState: %@", error.localizedDescription);
      exit(1);
   }
}
/*
 * Once per frame, fill the vertices array with either a sine wave (if testing)
 * or data from the ring_buffer which is holding the serial input
 */ 
void updateAllVertices(ring_buffer *rb){
   //printf("number of values in ring buffer is %d\n", rb->curr_num_values);

   static time_t last_time = 0;  // for measuring read rate
   static int test_counter = 0;           // for measuring read rate
   int num_avail_points = rb->curr_num_values;
   int num_to_plot = (num_avail_points < NUM_POINTS) ? num_avail_points : NUM_POINTS;

   if (last_time == 0){
      last_time = time(NULL);
   }

   for (int i = 0; i < num_to_plot; i++){
      float y = 0.0f;
      ring_buffer_read(rb, &y);
      test_counter++;
     // printf("[VISUALIZER] value read from ring buffer is %.6f\n", y);
      float x = (float)i / (NUM_POINTS - 1) * 2.0f - 1.0f; // -1 to +1
      //float x = (float)i / (NUM_POINTS - 1);             // 0 to +1
      //y = 0.5f * sinf(6.0f * (x + phase)); // sine wave
      //y = display_buffer[i];
      vertices[i].position = (vector_float2){ x, y};

      time_t now = time(NULL);
      if (now > last_time){
//         printf("[VISUALIZER] Floats read per second: %d\n", test_counter);
         test_counter = 0;
         last_time = now;
      }
  
   }
}

/*
 * Once per frame, change one vertex in the array with either a sine wave (if testing)
 * or data from the ring_buffer which is holding the serial input
 * This siumlates a point scrolling to the right and wrapping around, with all 
 * other points in the frame persisting. 
 */ 
void updateOneVertex(ring_buffer *rb){
   //printf("number of values in ring buffer is %d\n", rb->curr_num_values);

   // index of vertex to be drawn, needs to persist across frames
   static int draw_index = 0;
   float y = 0.0f;

   if (ring_buffer_read(rb, &y)) {
      // printf("[VISUALIZER] value read from ring buffer is %.6f\n", y);
      float scale_x = 1.0;
      //float y_normalized = (y - 3.0f) / 3.0f; 
      float y_normalized = (y - 1.0f) / 1.0f; 
      float x = (float)draw_index / (NUM_POINTS - 1) * 2.0f - 1.0f; // -1 to +1
//      x *= scale_x;
      //float x = (float)draw_index / (NUM_POINTS - 1) * 3.0f - 1.0f; // -1 to +1
      vertices[draw_index].position = (vector_float2){ x, y_normalized};
      draw_index = (draw_index + 1) % NUM_POINTS; // wraparound
      if (num_points_filled < NUM_POINTS){
         num_points_filled++;
      }
   }
}

void drawFrame(){
   id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
   if(!drawable) return;
  
   if(!pipelineState){
      NSLog(@"❌ pipelineState is nil - can't render.");
      exit(1);
   } 
   MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
   passDescriptor.colorAttachments[0].texture = drawable.texture;
   passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear; 
   // black background
   passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 1); 
   passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
   // transient and do not support reuse
   // in a typical app, an entire frame of rendering is encoded into a single command buffer
   id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
   id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

   [encoder setRenderPipelineState:pipelineState];
   [encoder setVertexBytes:vertices length:sizeof(vertices) atIndex:0];
   //[encoder setVertexBytes:reordered_vertices length:sizeof(reordered_vertices) atIndex:0];
   // where green sine wave gets drawn
   //[encoder drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:0 vertexCount:NUM_POINTS];
   [encoder drawPrimitives:MTLPrimitiveTypeLineStrip vertexStart:0 vertexCount:num_points_filled];
   [encoder endEncoding];

   [commandBuffer presentDrawable:drawable];
   [commandBuffer commit];
}

void start_visualization(ring_buffer *rb) {
   @autoreleasepool {
      initMetal();
      // create window with menu 
      NSApplication *app = [NSApplication sharedApplication];
      // Show dock icon + menu bar
      [app setActivationPolicy:NSApplicationActivationPolicyRegular];  
      // menubar with 'quit' option
      NSMenu *menubar = [[NSMenu alloc] init];
      NSMenuItem *appMenuItem = [[NSMenuItem alloc] init];
      [menubar addItem:appMenuItem];
      [app setMainMenu:menubar];
      NSMenu *appMenu = [[NSMenu alloc] init];
      NSMenuItem *quitMenuItem = [[NSMenuItem alloc]
          initWithTitle:@"Quit EEG Visualizer"
                 action:@selector(terminate:)
          keyEquivalent:@"q"];
      [appMenu addItem:quitMenuItem];
      [appMenuItem setSubmenu:appMenu];
      // main window
      NSWindow *window = [[NSWindow alloc] 
                          initWithContentRect:NSMakeRect(0, 0, WIDTH, HEIGHT)
                                   styleMask:(NSWindowStyleMaskTitled |
                                              NSWindowStyleMaskClosable |
                                              NSWindowStyleMaskResizable)
                                    backing:NSBackingStoreBuffered
                                      defer:NO];
      [window setTitle:@"EEG Visualizer"];
      [window makeKeyAndOrderFront:nil];
      [window.contentView setLayer:metalLayer];
      [window.contentView setWantsLayer:YES];
      [NSApp activateIgnoringOtherApps:YES]; // bring to front 
      // animation loop
      NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:1.0/12.0
                                                        repeats:YES
                                                        block:^(NSTimer * _Nonnull time){
         if (!keep_running){
            [NSApp terminate:nil];
         }
//         printf("[CALLING UPDATE VERTICES]\n");
         //updateAllVertices(rb);
         updateOneVertex(rb);
         drawFrame();
      }];

      [app run];
   }
}


