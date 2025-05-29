/**
 * ring_buffer.c
 *
 * Implementation of a fixed-size ring (circular) buffer for floating-point data streams.
 * This file provides the core logic for writing to and reading from the buffer.
 * 
 * Notes:
 * - Head and tail indices wrap around using modulo arithmetic.
 * - The buffer prevents overwrites when full (non-overwriting mode).
 * - Use with ring_buffer.h to access the public API.
 *
 * Typical usage:
 *   - Initialize with ring_buffer_init()
 *   - Write with ring_buffer_write()
 *   - Read with ring_buffer_read()
 *   - Destroy/free memory with ring_buffer_destroy()
 *
 * Author: Catherine Bernaciak PhD
 * Date: March (unthreaded), April (locks for threading) 2025
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "ring_buffer.h"

/**
 * Allocates memory for a new ring buffer with the specified capacity.
 * Lock for ring_buffer is initialized. 
 * 
 * rb is pointer to the ring buffer instance.
 * capacity is maximum number of values to store in the buffer.
 * returns void
*/
bool ring_buffer_init(ring_buffer *rb, int capacity){
   // setup the lock
   pthread_mutex_init(&rb->lock, NULL);

   if(capacity <= 0){
      return false; 
   }
   rb->max_num_values = capacity;
   rb->buffer = malloc(sizeof(float)*rb->max_num_values);
   if(!rb->buffer) return false; // occurs if insufficient memory

   rb->head = 0;
   rb->tail = 0;
   rb->curr_num_values = 0;
   return true;
}  

/**
 * Check if the ring buffer is empty.
 * Note: this is an internal function not to be used
 * outside of ring_buffer_read so as not to incur a 
 * race condition
 *
 * rb is pointer to the ring buffer instance.
 * return true if the buffer is empty, false otherwise.
 */
bool ring_buffer_empty(ring_buffer *rb){
   return(rb->curr_num_values == 0);
}

/**
 * Check if the ring buffer is full.
 * Note: this is an internal function not to be used
 * outside of ring_buffer_write so as not to incur a 
 * race condition
 *
 * rb is pointer to the ring buffer instance.
 * return true if the buffer is full, false otherwise.
 */ 
bool ring_buffer_full(ring_buffer *rb){
   return(rb->curr_num_values == rb->max_num_values);
}                

/**
 * Write a float value to the tail of the ring buffer.
 *
 * rb is pointer to the ring buffer instance.
 * value is the float value to be written into the buffer at 
 * the tail location, tail is then incremented.
 * return true on successful write, false if the buffer is full.
 */

bool ring_buffer_write(ring_buffer *rb, float32_t value){
   int retries = 0;
   // buffer is locked unless a deadlock arises
   while (pthread_mutex_trylock(&rb->lock) != 0){
      retries++;
      if (retries >= 100000){
         fprintf(stderr, "[WRITE] Mutex timeout! Possible deadlock.\n");
         return false;
      }
      if (retries % 1000 == 0){
         printf("[READ] .. waiting for lock after %d retries \n", retries);
      }
      usleep(10); // give other threads a chance
   }
   // lock the buffer
   //pthread_mutex_lock(&rb->lock);

   // test if buffer is full
   if(ring_buffer_full(rb)){
      // unlock rb so it can be read from elsewhere
      pthread_mutex_unlock(&rb->lock);
      return false;
   }
   // if not, add value to tail 
   rb->buffer[rb->tail] = value;
   rb->curr_num_values++;
   // tail increments or wraps around
   rb->tail = (rb->tail + 1) % rb->max_num_values; 

   // unlock rb so it can be written to elsewhere
   pthread_mutex_unlock(&rb->lock);  
   return true;
}

/**
 * Write a float value to the tail of the ring buffer, allow for overwrite.
 *
 * rb is pointer to the ring buffer instance.
 * value is the float value to be written into the buffer at 
 * the tail location, tail is then incremented.
 * There will always be a successful write, since we overwrite if
 * the buffer is full, so return true signifies something was written,
 * return false signifies the thread is locked and the buffer cannot
 * be accessed. 
 */

bool ring_buffer_overwrite(ring_buffer *rb, float32_t value){
   int retries = 0;

   // buffer is locked unless a deadlock arises
   while (pthread_mutex_trylock(&rb->lock) != 0){
      retries++;
      if (retries >= 100000){
         fprintf(stderr, "[WRITE] Mutex timeout! Possible deadlock.\n");
         return false;
      }
      if (retries % 1000 == 0){
         printf("[READ] .. waiting for lock after %d retries \n", retries);
      }
      usleep(10); // give other threads a chance
   }
   // lock the buffer
   //pthread_mutex_lock(&rb->lock);

   // overwrite if buffer is full
   if(ring_buffer_full(rb)){
      // unlock rb so it can be read from elsewhere
      pthread_mutex_unlock(&rb->lock);
      return false;
   }
   // if not, add value to tail 
   rb->buffer[rb->tail] = value;
   rb->curr_num_values++;
   // tail increments or wraps around
   rb->tail = (rb->tail + 1) % rb->max_num_values; 

   // unlock rb so it can be written to elsewhere
   pthread_mutex_unlock(&rb->lock);  
   return true;
}

/**
 * Read a value from the head of the ring buffer.
 *
 * rb is pointer to the ring buffer instance.
 * result is a pointer to the float value to be read
 * at head and then function returns true. 
 * Head is incremented and wraps around if necessary. 
 * If empty, returns false 
 */
bool ring_buffer_read(ring_buffer *rb, float *result){
   int retries = 0;

   // buffer is locked unless a deadlock arises
   while (pthread_mutex_trylock(&rb->lock) != 0){
      retries++;
      if (retries >= 100000){
         fprintf(stderr, "[READ] Mutex timeout! Possible deadlock.\n");
         return false;
      }
      if (retries % 1000 == 0){
         printf("[READ] .. waiting for lock after %d retries \n", retries);
      }
      usleep(10); // give other threads a chance
   }

   // test if (locked) buffer is empty, 
   if(ring_buffer_empty(rb)){ 
      // unlock rb so it can be written to elsewhere
      pthread_mutex_unlock(&rb->lock);
      return false;
   }

   // if not empty , return value from head
   *result = rb->buffer[rb->head];
   rb->curr_num_values--;
   // head increments or wraps around
   rb->head = (rb->head + 1) % rb->max_num_values;

   // unlock rb so it can be written to elsewhere
   pthread_mutex_unlock(&rb->lock);  
   return true;
}

/**
 * Free the allocated memory from the ring buffer.
 *
 * rb is pointer to the ring buffer instance.
 * return void 
 */
void ring_buffer_destroy(ring_buffer *rb){
   if (!rb) return; // if already null, nothing to do

   free(rb->buffer);
   rb->buffer = NULL; // safety
   pthread_mutex_destroy(&rb->lock); // free lock
   free(rb); 
   // can't set rb = NULL here bc NULL is passed in
   // setting rb = NULL is defined in a macro wherever
   // ring_buffer_destroy is called. 
}













