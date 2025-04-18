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
      usleep(args->read_rate); 
   }
   return NULL;
}

/*
 * void thread_pressure()
 * This a stress test for the concurrent ring buffer. It allows
 * for constant rw (basic), faster writes than reads (backpressure), 
 * faster reads than writes (negative backpressure).  Rate is set
 * by two inputs, which represent the integer number of microseconds 
 * delay between reads or writes. Ring buffer is allocated and destroyed 
 * within this function. There is wraparound. 
 *
 * inputs int write_rate, int read_rate
 * returns void
*/
void thread_pressure(int write_rate, int read_rate){

   if (write_rate == read_rate){
      printf("[TEST] Basic Concurrency Check ... \n");
   } else if (write_rate > read_rate){
      printf("[TEST] Concurrent Negative Backpressure Check ... \n");
   } else {
      printf("[TEST] Concurrent Backpressure Check ... \n");
   }

   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->write_rate = write_rate; // # us between writes
   thargs->read_rate = read_rate;   // # us between reads

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

int main(){
   thread_pressure(100, 100);
   thread_pressure(50, 100);
   thread_pressure(100, 50);
   //thread_basic_concurrency();
   //thread_backpressure();

   return 0;
}
