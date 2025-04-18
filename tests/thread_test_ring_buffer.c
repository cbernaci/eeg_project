/**
 * @file thread_test_ring_buffer.c
 * @brief Tests for the threaded ring_buffer library.
 *
 * This file contains a set of threaded integration tests to verify the 
 * correctness of concurrent functionality. We need to verify that concurrent
 * access from separate producer and consumer threads behaves properly: no
 * crashes, dropped values, or memory leaks.  These tests involve 2 or more 
 * threads accessing the same buffer concurrently. Concurrency test typs include:
 * - Basic Concurrency Check - constant but different rates of read & write
 * - Stress Tests:
 *      - Backpressure - high-frequency producer, slow consumer
 *      - Negative backpressure - slow producer, high-frequency consumer
 *      - Long-running wraparound 
 *      - Jittery producer and consumer 
 *      - Multiple producers and consumers
 * - Edge Test:
 *      - Mutex starvation / deadlock detection
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

typedef struct {
   ring_buffer *rb;
   float *written_vals;
   float *read_vals;
   int read_rate;
   int write_rate;
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
  
   // RHS = cast input void *arg to a thread_args* type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;

   // write continuously on this thread
   for (int i = 0; i < NUM_WRITES; i++){
      float val = (float)(rand() % 1000);
      if (ring_buffer_write(args->rb, val)){
         args->written_vals[i] = val;
      } 
      usleep(args->write_rate); 
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

   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   float read_val;

   // perform reads on consumer thread
   for (int i = 0; i < NUM_WRITES; i++){
      if (ring_buffer_read(args->rb, &read_val)){
         args->read_vals[i] = read_val;
      }
      usleep(args->read_rate); 
   }
   return NULL;
}

/*
 * void thread_basic_concurrency()
 * Tests that ring buffer can handle a large number of concurrent
 * writes and reads. The rate of writes and reads is equal. 
 * Ring buffer is allocated and destroyed within this function.
 * There is wraparound. 
 *
 * input void
 * returns void
*/
void thread_basic_concurrency(){
   printf("[TEST] Basic Concurrecy Check ... \n");
   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->write_rate = 0;
   thargs->read_rate = 0;

   // some simple tests
   assert(thargs != NULL);
   assert(thargs->rb != NULL);
   assert(thargs->written_vals != NULL);
   assert(thargs->read_vals != NULL);
   assert(ring_buffer_init(thargs->rb, BUFFER_CAPACITY));

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
/*
 * void thread_backpressure()
 * Tests that ring buffer can handle a large number of concurrent
 * writes and reads where there are faster writes than reads.
 * Ring buffer is allocated and destroyed within this function. 
 * There is wraparound. 
 *
 * input void
 * returns void
*/
void thread_backpressure(){

   printf("[TEST] Concurrent Backpressure Check ... \n");
   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->write_rate = 0;
   thargs->read_rate = 50;

   // some simple tests
   assert(thargs != NULL);
   assert(thargs->rb != NULL);
   assert(thargs->written_vals != NULL);
   assert(thargs->read_vals != NULL);
   assert(ring_buffer_init(thargs->rb, BUFFER_CAPACITY));

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
     printf("i ====================== %d\n", i);
     printf("written_vals[i] = %f\n", thargs->written_vals[i]);
     printf("read_vals[i] = %f\n", thargs->read_vals[i]);
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
   thread_basic_concurrency();
   thread_backpressure();

   return 0;
}
