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
#include <unistd.h>
#include "ring_buffer.h"
#include "test_helpers.h"

#define BUFFER_CAPACITY 10000
#define NUM_WRITES 1000000
#define BURST_SIZE 1000

/**
 * void stress_balanced_rw()
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

/**
 * void stress_burst_writes()
 * This mimics what might happen if data source buffers and dumps.
 * Tests that ring buffer can handle bursts of O(1000) writes with 
 * O(100) reads whenever buffer fills. There is a sleep period between
 * the bursts. For an attempted O(10^6) writes there should be O(10^4) 
 * write failures. 
 * Ring buffer is allocated and destroyed within this function.
 *
 * input void
 * returns void
*/
void stress_burst_writes(void){
   printf("[TEST] Burst writes ... \n");

   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   const int NUM_BURSTS = NUM_WRITES / BURST_SIZE;

   int write_attempts = 0;
   int write_successes = 0;
   int write_failures = 0;
   float dummy_read;

   for (int b = 0; b < NUM_BURSTS; b++){
      for (int i = 0; i < BURST_SIZE; i++){
         float val = (float)(b*BURST_SIZE + i);
         write_attempts++;

         // attempt write
         bool success = ring_buffer_write(rb, val);

         if (!success){
            write_failures++;

            // simulate some reads occuring
            for (int r = 0; r < BURST_SIZE/10; r++){
               ring_buffer_read(rb, &dummy_read);
            }

            // retry
            success = ring_buffer_write(rb, val);
            if (!success) { // this never happens actually
               write_failures++;
               printf(" double fail\n");
            }  
         } else {
            write_successes++;
         }

      usleep(1); // 0.001 ms delay
      }
   }

   printf("    Write attempts : %d\n", write_attempts);
   printf("    Write successes: %d\n", write_successes);
   printf("    Write failures : %d\n", write_failures);
   printf("    ✅ Passed (if no assertion failed)\n\n");

   SAFE_DESTROY(rb);
   printf("OK\n");
}

/**
 * void stress_jittery_input()
 * Alternates between fast and slow writes to model inconsistent serial
 * data. Ensures the buffer doesn't lose data or behave weirdly with 
 * irregular timing. One value is read if a write fails.
 * Size of buffer is 10K and the number of writes is 1M
 * so there will be wraparound. Sleep time is random to test true jitter. 
 * Ring buffer is allocated and destroyed within this function.
 *
 * input void
 * returns void
*/
void stress_jittery_input(void){
   printf("[TEST] Jittery input ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   float value;
   bool fast = 1;
   float dummy_read;
   int data_lost = 0;
   int attempted_writes = 0;
   int num_reads = 0;
   int num_writes = 0;

   // loop for writing
   for (int i = 0; i < NUM_WRITES; i++){
      value = sinf(i * 0.01f);  
      attempted_writes++;

      // fast writes
      if (fast){
         if(!ring_buffer_write(rb, value)){ // if full
            ring_buffer_read(rb, &dummy_read); // read a value
            num_reads++;
            assert(ring_buffer_write(rb, value)); // try again
            num_writes++;
         } else {
            num_writes++;
         }
      }
      // slow writes
      else { 
         if(!ring_buffer_write(rb, value)){ // if full
            ring_buffer_read(rb, &dummy_read); // read a value
            num_reads++;
            assert(ring_buffer_write(rb, value)); // try again
            num_writes++;
         } else {
            num_writes++;
            // random wait time between 0-99 µs
            usleep(rand() % 100);  
         }
      }
      // check if data lost
      int expected_buffer_size = num_writes - num_reads;
      if(rb->curr_num_values != expected_buffer_size){
         data_lost++;
      }
      // every 100 writes, switch write speed
      if((i + 1) % 100 == 0){
         fast ^= 1;
      }
   }
   printf("    Num attempted writes: %d\n", attempted_writes);
   printf("    Num actual writes: %d\n", num_writes);
   printf("    Num values written: %d\n", rb->curr_num_values);
   printf("    Num values read: %d\n", num_reads);
   printf("    Num values lost : %d\n", data_lost);
   printf("    ✅ Passed (if no assertion failed)\n\n");

   SAFE_DESTROY(rb);
   printf("OK\n");
}

/**
 * void stress_long_wraparound()
 * Performs 100 wraparounds on a buffer of size 10K. Everytime buffer fills
 * all values are read and verified. Simulates long-running behavior. The
 * ring buffer is allocated and destroyed within this function.
 *
 * input void
 * returns void
*/
void stress_long_wraparound(){
   printf("[TEST] Long wraparound ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   float write_value;
   float all_values[NUM_WRITES];
   float read_value;

   for (int i = 0; i < NUM_WRITES; i++){

      if (!ring_buffer_full(rb)){
         // get random value to write and store in array 
         // for later validation 
         write_value = rand();
         all_values[i] = write_value;
         // write value since buffer is not full yet
         assert(ring_buffer_write(rb, write_value)); 
      }
      else {
         assert(rb->head == rb->tail);
         // buffer is full, read all values and check
         // for correctness
         for (int j = 0; j < BUFFER_CAPACITY; j++){
            ring_buffer_read(rb, &read_value); 
            int idx = i - BUFFER_CAPACITY + j;
            ASSERT_FLOAT_EQ(read_value, all_values[idx]);
         }
         assert(ring_buffer_empty(rb));
         assert(rb->head == rb->tail);
      } 
   } 
   SAFE_DESTROY(rb);
   printf("OK\n");
}

/**
 * void stress_full_pressure()
 * Fills the buffer, then reads a value and writes a new one, checking
 * for correctness of the read value. There is wraparound. The test
 * is to see if while maintaining full pressure on the buffer there
 * is no anomalous behavior: lost values, etc. There are 1M writes on a 
 * buffer with capacity 10K. So there are O(1M) read checks. The ring
 * buffer is allocated and destroyed within this function.
 *
 * input void
 * returns void
*/
void stress_full_pressure(){
   printf("[TEST] Full pressure ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   float write_value;
   float all_values[NUM_WRITES];
   float read_value;

   for (int i = 0; i < NUM_WRITES; i++){

      if (!ring_buffer_full(rb)){
         // get random value to write and store in array 
         // for later validation 
         write_value = rand();
         all_values[i] = write_value;
         // write value since buffer is not full yet
         assert(ring_buffer_write(rb, write_value)); 
      }
      else {
         assert(rb->head == rb->tail);
         // buffer is full, read first value and check
         // for correctness
         ring_buffer_read(rb, &read_value); 
         int idx = i - BUFFER_CAPACITY; 
         ASSERT_FLOAT_EQ(read_value, all_values[idx]);
         // now write one new value
         write_value = rand();
         all_values[i] = write_value;
         assert(ring_buffer_write(rb, write_value));
      } 
   } 
   SAFE_DESTROY(rb);
   printf("OK\n");
}

/**
 * void stress_backpressure()
 * Simulates a faster write than read rate, to mimic fast writing and 
 * slower DSP processing behavior. This tests that write failures do not 
 * cause a crash. Buffer capacity is 10K, number of writes is 1M, read rate 
 * is a quarter of the write rate. Read value is checked for correctness.
 * There will be wraparound.
 *
 * input void
 * returns void
*/
void stress_backpressure(){
   printf("[TEST] Backpressure ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));
   //assert(ring_buffer_init(rb, 5));

   float write_value;
   float all_values[NUM_WRITES];
   //float all_values[10];
   float read_value;
   int m = 0; 
   for (int i = 0; i < NUM_WRITES; i++){
   //for (int i = 0; i < 10; i++){
      //printf("i,m =========================== %d,%d\n", i,m); 
      // if not full, write value, read at half rate
      if(!ring_buffer_full(rb)){
         write_value = rand();
         all_values[m] = write_value;
         //printf("all_values[i] = %f\n", all_values[m]);
         assert(ring_buffer_write(rb, write_value));

         if (m % 4 == 0) {
            ring_buffer_read(rb, &read_value);
            int idx = m/4; 
            /*
            printf("reading, at 1/4 rate and comparing the values ............\n");
            printf("read_value = %f\n", read_value);
            printf("all_values[idx] = %f\n", all_values[idx]);
            */
            ASSERT_FLOAT_EQ(read_value, all_values[idx]);
         }
         m++;
      }
      else { // if full, empty it completely, validating for correctness
          for (int j = 0; j < BUFFER_CAPACITY; j++){
          //for (int j = 0; j < 5; j++){
             ring_buffer_read(rb, &read_value);
             int idx = m - BUFFER_CAPACITY + j;
             /*
             int idx = m - 5 + j;
             printf("emptying buffer ............\n");
             printf("j = %d\n", j);
             printf("idx = %d\n", idx);
             printf("read_value = %f\n", read_value);
             printf("all_values[idx] = %f\n", all_values[idx]);
             */
             ASSERT_FLOAT_EQ(read_value, all_values[idx]); 
          }
          m = 0;
      }
   }
   SAFE_DESTROY(rb);
   printf("OK\n");
}
/*
 * void stress_negative_backpressure()
 * Simulates a faster read than write rate, to mimic fast processing and 
 * slower data input. This tests that read failures do not cause a crash
 * Start's from a partially filled buffer else it will just read and write
 * continuously (that behavior was tested above in 'stress_balanced_rw').
 * Buffer capacity is 10K, and is initialy written with 100 values. The 
 * total number of writes is 1M + 100, write rate is a quarter of the read
 * rate. Read values are checked for correctness. There is no wraparound.
 *
 * input void
 * returns void
*/
void stress_negative_backpressure(){
   printf("[TEST] Negative backpressure ... \n");
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(rb);
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));
   //assert(ring_buffer_init(rb, 5));


   int INITIAL_FILL = 100;
   //int INITIAL_FILL = 5;
   //int _NUM_WRITES = 10;
   float write_value;
   float all_values[NUM_WRITES + INITIAL_FILL];
   //float all_values[_NUM_WRITES + INITIAL_FILL];
   float read_value;
   int m = 0;
   int r = 0;

   // prefill
   for (int i = 0; i < INITIAL_FILL; i++){
      write_value = rand();
      all_values[i] = write_value;
      assert(ring_buffer_write(rb, write_value)); 
   }

   // begin main read/write loop
   for (int j = 0; j < NUM_WRITES; j++){
   //for (int j = 0; j < _NUM_WRITES; j++){
   
      if(!ring_buffer_empty(rb)){ // if not empty, read
         ring_buffer_read(rb, &read_value);
         //printf("r = %d\n", r);
         ASSERT_FLOAT_EQ(read_value, all_values[r]);
         r++;
      }

      if(j % 4 == 0){ // write at quarter rate of reading
         write_value = rand();
         assert(ring_buffer_write(rb, write_value));
         int idx = m + INITIAL_FILL;
         all_values[idx] = write_value;
         m++;
      } 
   }

   SAFE_DESTROY(rb);
   printf("OK\n");
}
/*
 * void stress_oscillating_rates()
 * Simulates oscillating read and write rates. I.e. simulate the writer
 * being faster at first, then the reader catching up, then falling behind
 * again. Test begins with a faster write rate, then switches to faster read
 * then does this switch one more time. Each time the buffer is written
 * to 1M times. Buffer capacity is 10K. This test calls other tests.
 * Read values are checked for correctness. There is wraparound.
 *
 * input void
 * returns void
*/
void stress_oscillating_rates(){
   printf("[TEST] Oscillating rates ............ \n");

   // 1. write 4 times as fast for the total num_writes
   stress_backpressure();
   // ...

   // 2. read 4 times as fast for the total num_writes
   stress_negative_backpressure();
   // ...

   // 3. write 4 times as fast for the total num_writes
   stress_backpressure();
   // ...

   // 4. read 4 times as fast for the total num_writes
   stress_negative_backpressure();

   printf("OK\n");
}
int main(){
   stress_balanced_rw();
   stress_burst_writes();
   stress_jittery_input();
   stress_long_wraparound();
   stress_full_pressure();
   stress_backpressure();
   stress_negative_backpressure();
   stress_oscillating_rates();
   return 0;
}
