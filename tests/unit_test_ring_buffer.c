/**
 * @file test_ring_buffer.c
 * @brief Unit tests for the ring_buffer library.
 *
 * This file contains a set of isolated unit tests to verify the correctness
 * of core ring buffer functionality, including:
 * - Initialization
 * - Read/write operations
 * - Empty and full state detection
 * - Basic wrap-around behavior
 *
 * Tests are run using simple assert() statements.
 *
 * Notes:
 * - These are *unit tests* (small, fast, and isolated from external dependencies).
 * - These tests do not simulate multithreaded or stress scenarios (see stress tests).
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

int main() {
   printf("//////// Running ring buffer unit tests ////////// \n");

   printf("....... empty buffer tests ....... \n");
   // initialize buffer
   ring_buffer *rb = malloc(sizeof(ring_buffer));
   ring_buffer_init(rb, 4);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->max_num_values == 4);
   assert(rb->curr_num_values == 0);

   // test empty buffer
   assert(ring_buffer_empty(rb) == true);
   assert(ring_buffer_full(rb) == false);

   printf("....... write tests ....... \n");
   // write 4 values to ring buffer
   // .. write first value
   assert(ring_buffer_write(rb, 15.234) == true);
   assert(rb->head == 0);
   assert(rb->tail == 1);
   assert(rb->curr_num_values == 1);
   float expected = 15.234;
   float actual = rb->buffer[rb->head];
   assert(fabs(actual-expected) < EPSILON);

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

   printf("....... full buffer tests ....... \n");
   // try to overfill buffer
   assert(ring_buffer_write(rb, 21.104) == false);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 4);
   assert(ring_buffer_full(rb) == true);
   assert(ring_buffer_empty(rb) == false);

   printf("....... read tests ....... \n");
   // read 4 values to ring buffer
   // .. read first value
   expected = 15.234;
   actual = ring_buffer_read(rb);
   assert(fabs(actual-expected) < EPSILON);
   assert(rb->head == 1);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 3);

   // .. read second value
   expected = 15.017;
   actual = ring_buffer_read(rb);
   assert(fabs(actual-expected) < EPSILON);
   assert(rb->head == 2);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 2);

   // .. read third value
   expected = 15.120;
   actual = ring_buffer_read(rb);
   assert(fabs(actual-expected) < EPSILON);
   assert(rb->head == 3);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 1);

   // .. read fourth value
   expected = 15.136;
   actual = ring_buffer_read(rb);
   assert(fabs(actual-expected) < EPSILON);
   assert(rb->head == 0);
   assert(rb->tail == 0);
   assert(rb->curr_num_values == 0);
   assert(ring_buffer_full(rb) == false);
   assert(ring_buffer_empty(rb) == true);

   printf("////////// All ring buffer unit tests passed /////////// \n");


}
