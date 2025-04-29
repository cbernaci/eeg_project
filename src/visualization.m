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
#import  <objc/runtime.h>
#import <pthread.h>
#import <stdio.h>
#import <stdlib.h>
#import "ring_buffer.h"

void sine_wave_test(){



}

int main() {
   // get the default metal device
   id<MTLDevice> device = MTLCreateSystemDefaultDevice();

   if (device) {
      //printf("✅ Metal device created successfully: %s\n", [[device name] UTF8String]);
      printf("✅ Metal device created successfully\n");
   }
   else {
      printf(" ❌ Failed to create Metal device!\n");
   }




}


