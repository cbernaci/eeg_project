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

#define BUFFER_CAPACITY 100000

volatile sig_atomic_t keep_running = 1;
ring_buffer *eeg_buffer_global = NULL;

/*
 * This function handles interruptions in the 
 * serial data stream from keyboard. It also cleans 
 * up the heap. 
 */
void handle_sigint(int sig){
   (void)sig;
   keep_running = 0;
      if(eeg_buffer_global){
         SAFE_DESTROY(eeg_buffer_global);
      }
   printf("SIGINT received. Stopping ...\n");
}

/*
 * Thread for writing to ring buffer
 */
void *producer_thread(void *arg){
   ring_buffer *rb = (ring_buffer *)arg;
   //sine_data_stream(rb);
   //read_physionet_data(rb);
   read_serial_data(rb);
   //read_low_pass_data(rb);
   return NULL;
}

int main(){

   // 0. register signal handler for keyboard interrupts
   signal(SIGINT, handle_sigint); 

   // 1. setup and initialize a single ring buffer
   //ring_buffer *eeg_buffer = malloc(sizeof(ring_buffer));
   ring_buffer *eeg_buffer = malloc(sizeof(ring_buffer));
   //eeg_buffer_global = eeg_buffer;
   //ring_buffer_init(eeg_buffer_global, BUFFER_CAPACITY);
   ring_buffer_init(eeg_buffer, BUFFER_CAPACITY);

   // 2. start producer thread that writes to ring buffer
   printf("[START READ SERIAL THREAD] ================= \n");
   pthread_t prod;
   if (pthread_create(&prod, NULL, producer_thread, eeg_buffer) != 0){
      perror("Faled to create producer thread\n");
      exit(1);
   }

   // for write rate of 2kHz and buffer capacity of 10,000
   // wait 5s for buffer to fill 
   //usleep(5000000); // wait 5.0 s for buffer to fill up 

   // 3. start Metal + Appkit visualization that reads from buffer
   // note, this is using the main thread, no need to create one
   printf("[START VISUALIZATION THREAD] ================= \n");
   start_visualization(eeg_buffer);
   
   printf("[JOINING THREADS]\n");  
   // 4. tell main thread to wait until producer thread finishes
   pthread_join(prod, NULL);

   return 0;
}
