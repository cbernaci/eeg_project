/**
 * @file unit_test_ring_buffer.c
 * @brief Unit tests for the ring_buffer library.
 *
 * This file contains a set of isolated unit tests to verify the correctness
 * of core ring buffer functionality, including:
 * - Initialization
 * - Read/write operations
 * - Empty and full state detection
 * - Basic wrap-around behavior
 * - allocated memory is freed
 *
 * Tests are grouped into functional blocks and individually run using assert() statements.
 *
 * Notes:
 * - These are *unit tests* (small, fast, and isolated from external dependencies).
 * - These tests do not simulate edge, multithreaded, or stress scenarios 
 *
 * Author: Catherine Bernaciak PhD
 * Date: March 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "ring_buffer.h"
#include "test_helpers.h"

//#define EPSILON 0.00001f
//#define ASSERT_FLOAT_EQ(a,b) assert(fabs((a)-(b)) < EPSILON)

void test_initialization(ring_buffer *rb){
   printf("[TEST] Initialization ... \n");
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->max_num_values == 4);
   assert(rb->curr_num_values == 0);
   printf("OK\n");
}

void test_empty_not_full(ring_buffer *rb){
   printf("[TEST] Empty and not full ... \n");
   assert(ring_buffer_empty(rb) == true);
   assert(ring_buffer_full(rb) == false);
   printf("OK\n");
}

void test_write_until_full(ring_buffer *rb){
   printf("[TEST] write until full ... \n");
   // write 4 values to ring buffer
   // .. write first value
   assert(ring_buffer_write(rb, 15.234) == true);
   assert(rb->head == 0);
   assert(rb->tail == 1);
   assert(rb->curr_num_values == 1);

   // .. write second value
   assert(ring_buffer_write(rb, 15.017) == true);
   assert(rb->head == 0);
   assert(rb->tail == 2);
   assert(rb->curr_num_values == 2);

   // .. write third value
   assert(ring_buffer_write(rb, 15.120) == true);
   assert(rb->head == 0);
   assert(rb->tail == 3);
   assert(rb->curr_num_values == 3);

   // .. write fourth value
   assert(ring_buffer_write(rb, 15.136) == true);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 4);

   // try to overfill buffer
   assert(ring_buffer_write(rb, 21.104) == false);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 4);
   assert(ring_buffer_full(rb) == true);
   assert(ring_buffer_empty(rb) == false);
   printf("OK\n");
}

void test_read_until_empty(ring_buffer *rb){
   printf("[TEST] Read until empty ... \n");
   // read 4 values from ring buffer
   // ... read first value
   float value;
   bool ok = ring_buffer_read(rb, &value); 
   assert(ok);
   ASSERT_FLOAT_EQ(value, 15.234f);
   assert(rb->head == 1);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 3);

   // ... read second value
   ok = ring_buffer_read(rb, &value); 
   assert(ok);
   ASSERT_FLOAT_EQ(value, 15.017f);
   assert(rb->head == 2);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 2);

   // ... read third value
   ok = ring_buffer_read(rb, &value); 
   assert(ok);
   ASSERT_FLOAT_EQ(value, 15.120f);
   assert(rb->head == 3);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 1);

   // ... read fourth value
   ok = ring_buffer_read(rb, &value); 
   assert(ok);
   ASSERT_FLOAT_EQ(value, 15.136f);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 0);
   assert(ring_buffer_full(rb) == false);
   assert(ring_buffer_empty(rb) == true);
   printf("OK\n");
}

void test_wraparound(ring_buffer *rb){
   // note that buffer should have been emptied from full previous 
   // to this or else wraparound behavior is not being tested

   printf("[TEST] wraparound behavior ... \n");
   // now start writing to the emptied buffer to make
   // sure head and tail behave properly 
   // ... write first wrapped value 
   assert(ring_buffer_write(rb, 21.104) == true);
   assert(rb->head == 0);
   assert(rb->tail == 1);
   assert(rb->curr_num_values == 1);
   assert(ring_buffer_full(rb) == false);
   assert(ring_buffer_empty(rb) == false);

   // ... write second wrapped value 
   assert(ring_buffer_write(rb, 26.501) == true);
   assert(rb->head == 0);
   assert(rb->tail == 2);
   assert(rb->curr_num_values == 2);
   assert(ring_buffer_full(rb) == false);
   assert(ring_buffer_empty(rb) == false);

   // ... write third wrapped value 
   assert(ring_buffer_write(rb, 14.150) == true);
   assert(rb->head == 0);
   assert(rb->tail == 3);
   assert(rb->curr_num_values == 3);
   assert(ring_buffer_full(rb) == false);
   assert(ring_buffer_empty(rb) == false);

   // ... write fourth wrapped value 
   assert(ring_buffer_write(rb, 20.878) == true);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 4);
   assert(ring_buffer_full(rb) == true);
   assert(ring_buffer_empty(rb) == false);
   printf("OK\n");
}

void test_destroy(ring_buffer *rb){
   printf("[TEST] memory leak indirectly ... \n");
   SAFE_DESTROY(rb);          // buffer is gone from heap 
   SAFE_DESTROY(rb);          // safe, won't call destroy again
   printf("OK\n");
   printf("... Note!! to properly test for memory leaks, now run:\n");
   printf("make memcheck\n");
}

int main() {

   ring_buffer *rb = malloc(sizeof(ring_buffer));
   assert(ring_buffer_init(rb, 4) == true);

   // these tests must be run in order, they are dependent
   // commments indicate state of buffer after function call
   test_initialization(rb);   // empty buffer with capacity 4
   test_empty_not_full(rb);   // buffer unchanged 
   test_write_until_full(rb); // buffer full
   test_read_until_empty(rb); // buffer empty
   test_wraparound(rb);       // buffer full
   test_destroy(rb);          // buffer is gone from heap and
                              // calling ring_buffer_destory more
                              // than once won't cause problem
}
