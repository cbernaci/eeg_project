/**
 * @file integration_test_ring_buffer.c
 * @brief Integration tests for the threaded ring_buffer library.
 *
 * This file contains a set of threaded integration tests to verify the 
 * correctness of concurrent functionality for multiple, interacting 
 * ring buffers. We need to verify that concurrent access from separate
 * producer and consumer threads behaves properly when multiple ring 
 * buffers are interacting with each other. We are specifically testing
 * the scenario for the EEG where there would be 
 * Serial In → [ RB1 ] → Filter → [ RB2 ] → FFT → [ RB3 ] → Visualizer
 * and there would be no crashes, dropped values, or memory leaks.
 *
 * Author: Catherine Bernaciak PhD
 * Date: April 2025
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ring_buffer.h"
#include "test_helpers.h"

//#define NUM_WRITES 10000
#define NUM_WRITES 10 
//#define BUFFER_CAPACITY 1000
#define BUFFER_CAPACITY 4

int main(){

   // serial in -> RB1 -> filter
   // filter -> RB2 -> FFT
   // FFT -> RB3 -> Visualizer



}

