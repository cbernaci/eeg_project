 /*
 * @file ring_buffer.h
 * @brief Simple ring (circular) buffer implementation for real-time data streams.
 *
 * This header provides the API for a fixed-size, FIFO ring buffer.
 * It supports enqueueing (write) and dequeueing (read) of floating-point values
 * in constant time. The buffer automatically wraps around when it reaches the end.
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
 * Date: March 2025 
 */
 
// include guard
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>

// Custom typedef to make float size explicit
typedef float float32_t;

typedef struct {
   float32_t *buffer;
   int head;
   int tail; 
   int curr_num_values; 
   int max_num_values;
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
 * @return The float value read. If empty, returns QUEUE_EMPTY
 */
float ring_buffer_read(ring_buffer *rb);               

/**
 * @brief Check if the ring buffer is empty.
 *
 * @param rb Pointer to the ring buffer instance.
 * @return true if the buffer is empty, false otherwise.
 */
bool ring_buffer_empty(ring_buffer *rb);             

/**
 * @brief Check if the ring buffer is full.
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
