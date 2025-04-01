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
#include "ring_buffer.h"
#include "test_helpers.h"


int main(){

   stress_constant_throughput();
   stress_burst_writes();
   stress_jittery_input();
   stress_full_buffer_pressure();
   stress_long_wraparound();
   stress_backpressure();
   stress_oscillating_rates();
   stress_data_integrity();
   return 0;
}
