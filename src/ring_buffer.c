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
 * she's a gypsy and she's trouble
 * Date: March 2025
 */

#include "ring_buffer.h"
#include <stdlib.h>
#include <stdbool.h>

/**
 * Allocates memory for a new ring buffer with the specified capacity.
 * 
 * rb is pointer to the ring buffer instance.
 * capacity is maximum number of values to store in the buffer.
 * returns void
*/
void ring_buffer_init(ring_buffer *rb, int capacity){
   rb->max_num_values = capacity;
   rb->buffer = malloc(sizeof(float)*rb->max_num_values);
   rb->head = 0;
   rb->tail = 0;
   rb->curr_num_values = 0;
}  

/**
 * Check if the ring buffer is empty.
 *
 * rb is pointer to the ring buffer instance.
 * return true if the buffer is empty, false otherwise.
 */
bool ring_buffer_empty(ring_buffer *rb){
   return(rb->curr_num_values == 0);
}

/**
 * Check if the ring buffer is full.
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

   // test if buffer is full
   if(ring_buffer_full(rb)){return false;}
   // if not, add value to tail 
   rb->buffer[rb->tail] = value;
   rb->curr_num_values++;
   // tail increments or wraps around
   rb->tail = (rb->tail + 1) % rb->max_num_values; 
   return true;
}

/**
 * Read a value from the head of the ring buffer.
 *
 * rb is pointer to the ring buffer instance.
 * return the float value read from head, head
 * is then incremented and wraps around if necessary. 
 * If empty, returns QUEUE_EMPTY
 */
float ring_buffer_read(ring_buffer *rb){

   float result;
   // test if buffer is empty
   //if(ring_buffer_empty(rb)){return QUEUE_EMPTY;}
   if(ring_buffer_empty(rb)){return 42;}
   // if not, return value from head
   result = rb->buffer[rb->head];
   rb->curr_num_values--;
   // head increments or wraps around
   rb->head = (rb->head + 1) % rb->max_num_values;
   return result;
}

/**
 * Free the allocated memory from the ring buffer.
 *
 * rb is pointer to the ring buffer instance.
 * return void 
 */
void ring_buffer_destroy(ring_buffer *rb){
   free(rb->buffer);
   free(rb);
}













