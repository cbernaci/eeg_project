/**
 * @file thread_test_ring_buffer.c
 * @brief Tests for the threaded ring_buffer library.
 *
 * This file contains a set of threaded integration tests to verify the 
 * correctness of concurrent functionality. We need to verify that concurrent
 * access from separate producer and consumer threads behaves properly: no
 * crashes, dropped values, or memory leaks.  These tests involve 2 or more 
 * threads accessing the same buffer concurrently.
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

#define NUM_WRITES 10000
#define BUFFER_CAPACITY 10000

typedef struct {
   ring_buffer *rb;
   float *written_vals;
   float *read_vals;
} thread_args;

void *producer_thread(void *arg){
   //printf("Producer started \n");
  
   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;

   // write continuously on this thread
   for (int i = 0; i < NUM_WRITES; i++){
   //for (int i = 0; i < 10; i++){
      float val = (float)(rand() % 1000);
      if (ring_buffer_write(args->rb, val)){
         args->written_vals[i] = val;
         //printf("[Producer] wrote: %.2f\n", val);
      } 
      usleep(500); // sleep 0.5 s, simulating faster production
   }
   return NULL;
}

void *consumer_thread(void *arg){
   //printf("Consumer started \n");

   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   float read_val;

   // perform reads on consumer thread
   for (int i = 0; i < NUM_WRITES; i++){
   //for (int i = 0; i < 10; i++){
      if (ring_buffer_read(args->rb, &read_val)){
         args->read_vals[i] = read_val;
         //printf("[Consumer] read: %.2f\n", read_val);
      }
      usleep(1000); // sleep 1 s, simulating slower production
   }
   return NULL;
}


int main(){
   printf("[TEST] Basic Concurrecy Check ... \n");
   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);

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
   //for (int i = 0; i < 10; i++){
   for (int i = 0; i < NUM_WRITES; i++){
     assert(thargs->read_vals[i] == thargs->written_vals[i]);
   }

   // clean up the heap
   free(thargs->written_vals);
   free(thargs->read_vals);
   SAFE_DESTROY(thargs->rb);
   free(thargs);
   printf("OK\n");
   return 0;
}
