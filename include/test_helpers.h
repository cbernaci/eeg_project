/*
 * @file test_helpers.h
 * @brief define macro for testing or using the ring_buffer_destroy() function
 *
 * This macro should be used whenever ring_buffer_destroy() is called
 *
 * Author: Catherine Bernaciak PhD 
 * Date: March 2025 
 */



#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#define EPSILON 0.00001f
#define ASSERT_FLOAT_EQ(a,b) assert(fabs((a)-(b)) < EPSILON)

#define SAFE_DESTROY(rb_ptr)    \
   do {                         \
       if ((rb_ptr) != NULL){   \
           ring_buffer_destroy(rb_ptr); \
           rb_ptr = NULL;       \
       } \
   } while (0)

#endif
