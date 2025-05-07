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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//#include <stdbool.h>
#include <unistd.h>
#include "read_serial_data.h" // serial-data (producer thread)
#include "ring_buffer.h"      // thread-safe ring buffer
#include "visualization.h"    // metal+AppKit 

// setup ring buffer (for now just one) 
#define BUFFER_CAPACITY 1000
ring_buffer *eeg_buffer = malloc(sizeof(ring_buffer));
ring_buffer_init(eeg_buffer, BUFFER_CAPACITY);

void *producer_thread(void *arg){
   data_stream_init(&eeg_buffer);
   return NULL;
}

