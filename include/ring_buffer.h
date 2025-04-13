 /*
 * @file ring_buffer.h
 * @brief Thread-safe ring (circular) buffer implementation for real-time data streams.
 *
 * This header provides the API for a fixed-size, FIFO ring buffer.
 * It supports enqueueing (write) and dequeueing (read) of floating-point values
 * in constant time. The buffer automatically wraps around when it reaches the end.

 * The ring buffer supports concurrent access by a single producer thread and a single
 * consumer thread. Internally, it uses a `pthread_mutex_t` to ensure safe access to shared
 * state (head/tail pointers and data array). 
 *
 * Note: The ring buffer itself does not create or manage threads - it is the responsibility
 * of the caller to coordinate producer and consumer threads that operate on the buffer. 
 *
 * Usage:
 * - Initialize using `ring_buffer_init()`
 * - Write using `ring_buffer_write()`
 * - Read using `ring_buffer_read()`
 * - Free memory with `ring_buffer_destroy()`
 *
 * Functions returning `bool` will indicate:
 * - `true` = success or positive condition
 * - `false` = failure or negative condition
 *
 * Application:
 * - Real-time EEG data buffering
 *
 * Author: Catherine Bernaciak PhD 
 * Date: March 2025 (original) April 2025 (multithreading) 
 */
 
// include guard
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <pthread.h>

// Custom typedef to make float size explicit
typedef float float32_t;

/** 
* @struct ring_buffer
* @brief A thread-safe circular buffer of floats
*
* This struct contains internal state for managing a fixed-size circular buffer,
* including buffer pointer, head/tail indices, size counters, and a POSIX mutex.
*/
typedef struct {
   float32_t *buffer;     // pointer to the float array holding data
   int head;              // read index - updated by consumer
   int tail;              // write index - updated by producer
   int curr_num_values;   // number of values currently in the buffer
   int max_num_values;    // max capacity of the buffer
   pthread_mutex_t lock;  // mutex to guard access to the buffer state
} ring_buffer;

/**
 * @brief Initialize a ring buffer.
 * 
 * Allocates memory for a new ring buffer with the specified capacity.
 * 
 * @param rb Pointer to the ring buffer instance.
 * @param capacity Maximum number of values to store in the buffer.
 * @return void
*/
bool ring_buffer_init(ring_buffer *rb, int capacity);  

/**
 * @brief Write a float value into the ring buffer.
 *
 * @param rb Pointer to the ring buffer instance.
 * @param value The float value to be written into the buffer.
 * @return true on successful write, false if the buffer is full.
 */
bool ring_buffer_write(ring_buffer *rb, float32_t value);  

/**
 * @brief Read a value from the ring buffer.
 *
 * @param rb Pointer to the ringBuffer instance.
 * @param result Pointer to the float value that's read.
 * @return true is float value read, false if empty
 */
bool ring_buffer_read(ring_buffer *rb, float *result);               

/**
 * @brief Check if the ring buffer is empty.
 * Note: This function is not to be used outside of
 * ring_buffer_read, otherwise race conditions can occur.
 *
 * @param rb Pointer to the ring buffer instance.
 * @return true if the buffer is empty, false otherwise.
 */
bool ring_buffer_empty(ring_buffer *rb);             

/**
 * @brief Check if the ring buffer is full.
 * Note: This function is not to be used outside of
 * ring_buffer_write, otherwise race conditions can occur.
 *
 * @param rb Pointer to the ringBuffer instance.
 * @return true if the buffer is full, false otherwise.
 */ 
bool ring_buffer_full(ring_buffer *rb);                

/**
 * @brief Free the allocated memory from the ring buffer.
 *
 * @param rb Pointer to the ringBuffer instance.
 * @return void 
 */
void ring_buffer_destroy(ring_buffer *rb);             

#endif
