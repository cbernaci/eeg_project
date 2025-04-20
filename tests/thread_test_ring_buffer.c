/**
 * @file thread_test_ring_buffer.c
 * @brief Tests for the threaded ring_buffer library.
 *
 * This file contains a set of threaded integration tests to verify the 
 * correctness of concurrent functionality. We need to verify that concurrent
 * access from separate producer and consumer threads behaves properly: no
 * crashes, dropped values, or memory leaks.  These tests involve 2 or more 
 * threads accessing the same buffer concurrently. Concurrency tests are:
 * - Stress Tests:
 *    - Basic Concurrency Check - identical, constant rates of read & write
 *    - Backpressure - high-frequency producer, slow consumer
 *    - Negative backpressure - slow producer, high-frequency consumer
 *    - Long-running wraparound 
 *    - Jittery producer and consumer 
 *    - Multiple producers and consumers
 * - Edge Test:
 *    - Mutex starvation / deadlock detection
 * Author: Catherine Bernaciak PhD
 * Date: April 2025
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ring_buffer.h"
#include "test_helpers.h"

#define NUM_WRITES 10000
//#define NUM_WRITES 10 
#define BUFFER_CAPACITY 1000
//#define BUFFER_CAPACITY 4

typedef struct {
   ring_buffer *rb;
   float *written_vals;
   float *read_vals;
   int read_rate;
   int write_rate;
   bool jitter;
} thread_args;

/*
 * void *producer_thread()
 * Continously writes NUM_WRITES random values to the 
 * ring buffer on a separate thread. Stores values written
 * in a separate array for later comparison. Sleep time between 
 * writes is variable.  
 * 
 * input void pointer
 * returns void pointer
*/
void *producer_thread(void *arg){
   //printf("producer thread ==========\n"); 
   // RHS = cast input void *arg to a thread_args* type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   int write_count = 0;

   // write continuously on this thread
   while(write_count < NUM_WRITES){
      float val = (float)(rand() % 1000);
      if (ring_buffer_write(args->rb, val)){
         //printf("value written in producer thread: %f\n", val);
         args->written_vals[write_count] = val;
         write_count++;
      } 
      if (!args->jitter){
         usleep(args->write_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if write_rate = 9,000
         usleep(1000 + arc4random_uniform(args->write_rate)); 
      }
   }
   return NULL;
}

/*
 * void *consumer_thread()
 * Continously read NUM_WRITES random values from the 
 * ring buffer on a separate thread. Stores values read in 
 * a separate array for later comparison. Sleep time between
 * reads is variable. 
 *
 * input void pointer
 * returns void pointer
*/
void *consumer_thread(void *arg){

   //printf("consumer thread ==========\n"); 
   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   float read_val;
   int read_count = 0;

   // perform reads on consumer thread
   while(read_count < NUM_WRITES){
      if (ring_buffer_read(args->rb, &read_val)){
         //printf("value read in consumer thread: %f\n", read_val);
         args->read_vals[read_count] = read_val;
         read_count++;
      }
      if (!args->jitter){
         usleep(args->read_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if read_rate = 9,000
         usleep(1000 + arc4random_uniform(args->read_rate)); 
      }
   }
   return NULL;
}

/*
 * void thread_pressure()
 * This performs a set of stress tests for the concurrent ring buffer.
 * It allows for constant rw (basic), faster writes than reads
 * (backpressure), faster reads than writes (negative backpressure).  
 * Rate is set by two inputs, which, in the case of no jitter, represent
 * the integer number of microseconds delay between writes and reads. In 
 * the case of jitter, the inputs are 1000 us less than the max delay.
 * Ring buffer is allocated and destroyed within this function. There is 
 * a variable amount of wraparound to allow for long-running wraparound
 * (small capacity, many writes). 
 *
 * inputs int write_rate, int read_rate, int capacity
 * returns void
*/
void thread_pressure(int write_rate, int read_rate, int capacity, bool jitter){
   
   if(jitter) {
      printf("[TEST] Random Jitter ... \n");
   } else {
      if (capacity == BUFFER_CAPACITY){
         if (write_rate == read_rate){
            printf("[TEST] Basic Concurrency ... \n");
         } else if (write_rate > read_rate) {
            printf("[TEST] Concurrent Negative Backpressure ... \n");
         } else if (read_rate > write_rate){
            printf("[TEST] Concurrent Backpressure ... \n");
         }
      } else {
         printf("[TEST] Long running wraparound ... \n");
      }
   }

   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->write_rate = write_rate; // # us between writes
   thargs->read_rate = read_rate;   // # us between reads
   thargs->jitter = jitter;         // jitter Y/N

   // some simple tests
   assert(thargs != NULL);
   assert(thargs->rb != NULL);
   assert(thargs->written_vals != NULL);
   assert(thargs->read_vals != NULL);
   assert(ring_buffer_init(thargs->rb, capacity));

   // pthread_t is an opaque type, holds thread ID's assigned by OS
   pthread_t prod, cons;

   // start first thread
   pthread_create(&prod, NULL, producer_thread, thargs);
   // return to main thread and begin executing this one
   pthread_create(&cons, NULL, consumer_thread, thargs);

   // tell main thread to wait until these threads finish before continuing
   pthread_join(prod, NULL);
   pthread_join(cons, NULL);

   // test that all values written and read are the same - no loss
   for (int i = 0; i < NUM_WRITES; i++){
     //printf("i ====================== %d\n", i);
     //printf("written_vals[i] = %f\n", thargs->written_vals[i]);
     //printf("read_vals[i] = %f\n", thargs->read_vals[i]);
     assert(thargs->read_vals[i] == thargs->written_vals[i]);
   }

   // clean up the heap
   free(thargs->written_vals);
   free(thargs->read_vals);
   SAFE_DESTROY(thargs->rb);
   free(thargs);
   printf("OK\n");
}

int main(){
   // Basic Concurrency - even write & read, no jitter
   thread_pressure(100, 100, BUFFER_CAPACITY, 0); 
   // Backpressure - faster write than read, no jitter
   thread_pressure(50, 100, BUFFER_CAPACITY, 0);  
   // Negative Backpressure - faster read than write, no jitter
   thread_pressure(100, 50, BUFFER_CAPACITY, 0);  
   // Long running wrapaaround - even write, no jitter
   thread_pressure(100, 100, 50, 0);              
   // random jitter btwn ~1-30 ms for write and read 
   // pass in max delay rate for write & read
   thread_pressure(9000, 9000, BUFFER_CAPACITY, 1);              

   return 0;
}
