/**
 * @file stress_test_ring_buffer.c
 * @brief Stress tests for the ring_buffer library.
 *
 * This file contains a set of isolated stress tests that validate the performance
 * of the ring buffer under extreme conditons such as extremely large numbers of read
 * and writes, jittery, bursty, or otherwise fast, anomalous data. Stress test types 
 * include: 
 * - Constant throughput writing at high rate
 * - Bursty writing
 * - Jittery writing
 * - Long-running wraparound
 * - Several pressure tests:
 *    - Full pressure - read with nearly full buffer 
 *    - Backpressure - write faster than reading
 *    - Negative backpressure - read faster than writing
 * - Oscilattion producer/consumer rates
 * - Data Integrity - checksummed input
 *
 * Each tests gets its own function which allocates its own ring buffer, runs
 * it's pattern, asserts metrics and frees the buffer. 
 *
 * Author: Catherine Bernaciak PhD
 * Date: April 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "ring_buffer.h"
#include "test_helpers.h"

#define BUFFER_CAPACITY 10000
#define NUM_WRITES 1000000

/**
 * Tests that ring buffer can handle alternating writes and reads  
 * a very high number of times at a steady rate. Ring buffer is 
 * allocated and destroyed within this function.
 *
 * input void
 * returns void
*/
void stress_balanced_rw(void){
   printf("[TEST] Balanced Read/Write ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   float value;
   for (int i = 0; i < NUM_WRITES; i++){
      // write value to buffer
      assert(ring_buffer_write(rb, (float)i));
      // immediately read value back
      assert(ring_buffer_read(rb, &value));
      // check it's same value written and read
      ASSERT_FLOAT_EQ(value, (float)i);
   }
  
   SAFE_DESTROY(rb);
   printf("OK\n");
}

int main(){

   stress_balanced_rw();
   //stress_burst_writes();
   //stress_jittery_input();
   //stress_long_wraparound();
   //stress_full_pressure();
   //stress_backpressure();
   //stress_negative_backpressure();
   //stress_oscillating_rates();
   //stress_data_integrity();
   return 0;
}
