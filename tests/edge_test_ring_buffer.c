/**
 * @file edge_test_ring_buffer.c
 * @brief Edge case tests for the ring_buffer library.
 *
 * This file contains a set of isolated edge tests to stress the logic boundaries
 * of the ring buffer. It helps ensure the ring buffer is resilient under wierd 
 * or extreme inputs, including:
 * - Invalid initialization
 * - Writing to buffer with max_num_values = 1 
 * - Read from empty buffer 
 * - Alternating read/write pattern
 *
 * Tests are grouped into functional blocks and individually run using assert() statements.
 *
 * Author: Catherine Bernaciak PhD
 * Date: March 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "ring_buffer.h"

#define EPSILON 0.00001f
#define ASSERT_FLOAT_EQ(a,b) assert(fabs((a)-(b)) < EPSILON)

/**
 * Tests that false is returned (not an error) if a ring buffer is 
 * initialized with a capacity <= 0. 
 *
 * rb is pointer to the ring buffer instance.
 * returns void
*/
void test_invalid_init(ring_buffer *rb){
   
   printf("[TEST] Invalid initialization ... \n");
   assert(ring_buffer_init(rb, 0) == false);
   assert(ring_buffer_init(rb, -1) == false);
   printf("OK\n");
}

/**
 * Tests that false is returned (not an error) if a ring buffer  
 * with capactiy 1 is written to. 
 *
 * rb is pointer to the ring buffer instance.
 * returns void
*/
void test_buffer_size_1(ring_buffer *rb){
   printf("[TEST] Buffer size 1 ... \n");
   ring_buffer_init(rb, 1);
   assert(ring_buffer_write(rb, 3.141f) == true);
   assert(ring_buffer_write(rb, 1.618f) == false);
   printf("OK\n");
}

/**
 * Tests that read on empty ring buffer returns false, not an error
 * 
 * rb is pointer to the ring buffer instance. 
 * note that rb->curr_num_value = 1 on input! or else not testing
 * empty buffer read
 * returns void
*/
void test_empty_buffer_read(ring_buffer *rb){
   printf("[TEST] Empty buffer read ... \n");
   float value;
   // read a value, not it's empty
   bool ok = ring_buffer_read(rb, &value);
   assert(ok);
   ok = ring_buffer_read(rb, &value);
   assert(!ok);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   printf("OK\n");
}

/**
 * Tests 100 instances of alternating read and write of 
 * same value on full ring buffer of capacity 1.
 *
 * rb is pointer to the ring buffer instance.
 * returns void
*/
void test_alternating_rw(ring_buffer *rb){
   printf("[TEST] Alternating read then write ... \n");
   // write 4.0f to empty ring buffer
   float value = 4.0f;
   ring_buffer_write(rb, value);  // buffer has 1 value now
   assert(rb->curr_num_values == 1);
   assert(rb->max_num_values == 1);
   assert(ring_buffer_full(rb) == true);
   bool ok;
   int i;
   for (i = 0; i < 100; i++){
      // read and test
      ok = ring_buffer_read(rb, &value);
      assert(ok);
      assert(rb->head == 0);
      assert(rb->tail == 0);
      assert(rb->curr_num_values == 0);
      ASSERT_FLOAT_EQ(value, 4.0f);
      // write and test
      assert(ring_buffer_write(rb, 4.0f) == true);
      assert(rb->head == 0);
      assert(rb->tail == 0);
      assert(rb->curr_num_values == 1);
   }
   printf("OK\n");
}

int main() {
   ring_buffer *rb = malloc(sizeof(ring_buffer));

   // these tests must be run in order, they are dependent
   test_invalid_init(rb);
   test_buffer_size_1(rb); // ring buffer has 1 element and size 1 after this
   test_empty_buffer_read(rb); // ring buffer is empty now
   test_alternating_rw(rb);  
   free(rb);
}
