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
#import "ring_buffer.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#define WIDTH 800
#define HEIGHT 600
#define NUM_POINTS 512

// global Metal objects
id<MTLDevice> device;             // connection to GPU driver
id<MTLCommandQueue> commandQueue; // a queue of GPU commands
id<MTLRenderPipelineState> pipelineState; // info for GPU to draw 
CAMetalLayer *metalLayer;

typedef struct {
   vector_float2 position;
} Vertex;
Vertex vertices[NUM_POINTS];

// compile a simple shader
NSString* shaderSource = @"\
#include <metal_stdlib>                  \n\
using namespace metal;                   \n\
struct VertexIn{                         \n\
   float2 position [[attribute(0)]];     \n\
};                                       \n\
                                         \n\
vertex float4 vertex_main(VertexIn in [[stage_in]]) { \n\
   return float4(in.position, 0.0,1.0);  \n\
};                                       \n\
                                         \n\
fragment float4 fragment_main(){         \n\
   return float4(0.0,1.0); // green line \n\
}                                        \n\
";

float phase = 0.0f; // phase of sine wave

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


}

// fill the vertices array with a sine wave
void updateVertices(){}
void drawFrame(){}

int main() {
   @autoreleasepool {
      initMetal();
   }

   return 0;
}


