// main - App entry point (e.g., for real-time pipeline)
/**
 * ring_buffer.c
 *
 * This is the app entry point. This file reads the serial data stream from the 
 * microcontroller, passes it on a separate thread to a ring buffer, the main 
 * thread reads from the ring buffer and passes to Metal visualization functions
 * for real-time waveform viewing. 
 * 
 * Todo:
 * - Put in processing steps (preprocessing and DSP)
 * - Update visualization functions to have GUI be interactive and with more info
 *
 * Author: Catherine Bernaciak PhD
 * Date: May 2025
 */
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "eeg_config.h"       // configure buffers
#include "read_serial_data.h" // serial-data (producer thread)
#include "ring_buffer.h"      // thread-safe ring buffer
#include "visualization.h"    // metal+AppKit 
#include "test_helpers.h"

#define BUFFER_CAPACITY 10000


// thread for writing to ring buffer
void *producer_thread(void *arg){
   ring_buffer *rb = (ring_buffer *)arg;
   //sine_data_stream(rb);
   read_physionet_data(rb);
   return NULL;
}

int main(){
   // 1. setup and initialize a single ring buffer
   ring_buffer *eeg_buffer = malloc(sizeof(ring_buffer));
   ring_buffer_init(eeg_buffer, BUFFER_CAPACITY);

   // 2. start producer thread that writes to ring buffer
   pthread_t prod;
   if (pthread_create(&prod, NULL, producer_thread, eeg_buffer) != 0){
      perror("Faled to create producer thread\n");
      exit(1);
   }

     // this doesn't make a difference
//   usleep(13000000); // wait 13 s for buffer to fill up

   // 3. start Metal + Appkit visualization that reads from buffer
   // note, this is using the main thread, no need to create one
   start_visualization(eeg_buffer);
   
   // 4. tell main thread to wait until producer thread finishes
   pthread_join(prod, NULL);

   // 5. cleanup heap
   SAFE_DESTROY(eeg_buffer);
  
   return 0;
}
