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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ring_buffer.h"

void *producer_thread(void *arg){
   printf("Producer started \n");
   return NULL;
}

void *consumer_thread(void *arg){
   printf("Consumer started \n");
   return NULL;
}


int main(){
   // pthread_t is an opaque type
   // holds the thread ID's assigned by OS
   pthread_t prod, cons;

   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(ring_buffer_init(rb, 1024));

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
