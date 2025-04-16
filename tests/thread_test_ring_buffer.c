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

#define NUM_WRITES 10000
#define BUFFER_CAPACITY 10000

void *producer_thread(void *arg){
   printf("Producer started \n");
  
   // RHS = cast input void *arg to a ring_buffer * type
   // LHS = declare new variable rb that's a pointer to a ring_buffer
   ring_buffer *rb = (ring_buffer *)arg;
   float *written_values[NUM_WRITES];

   // write continuously on this thread
   //for (int i = 0; i < NUM_WRITES; i++){
   for (int i = 0; i < 10; i++){
      float val = rand();
      if (ring_buffer_write(rb, val)){
         *written_values[i] = val;
         printf("[Producer] wrote: %.2f\n", val);
      } else {
         printf("[Producer] Buffer full. Skipping write.\n");
      }
      usleep(500); // sleep 0.5 s, simulating faster production
   }
   return NULL;
}

void *consumer_thread(void *arg){
   printf("Consumer started \n");

   ring_buffer *rb = (ring_buffer *)arg;
   float read_val;
   float *read_values[NUM_WRITES];
  

   // perform 1000 reads on one thread
   //for (int i = 0; i < 1000; i++){
   for (int i = 0; i < 10; i++){
      if (ring_buffer_read(rb, &read_val)){
         *read_values[i] = read_val;
         printf("[Consumer] read: %.2f\n", read_val);
      } else {
         printf("[Consumer] Buffer empty. Waiting.\n");

      }
      usleep(1000); // sleep 1 s, simulating slower production
   }

   return NULL;
}


int main(){
   // pthread_t is an opaque type
   // holds the thread ID's assigned by OS
   pthread_t prod, cons;

   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(ring_buffer_init(rb, BUFFER_CAPACITY));

   /* int pthread_create(
   * pthread_t *thread,              // thread ID written here
   * const pthread_attr_t *attr,     // optional thread attributes (NULL = default)
   * void *(*start_routine)(void *), // function that the thread runs
   * void *arg);                     // argument passed to that function
   */
   // start first thread
   pthread_create(&prod, NULL, producer_thread, rb);
   // return to main thread and begin executing this one
   pthread_create(&cons, NULL, consumer_thread, rb);

   // tell main thread to wait until these threads finish before continuing
   pthread_join(prod, NULL);
   pthread_join(cons, NULL);

   return 0;
}
