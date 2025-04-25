/**
 * @file thread_test_ring_buffer.c
 * @brief Tests for the threaded ring_buffer library.
 *
 * This file contains a set of threaded integration tests to verify the 
 * correctness of concurrent functionality for individual ring buffers and
 * for several connected together (pipeline). These tests serve to verify that 
 * concurrent access from separate producer and consumer threads behave properly:
 * no crashes, dropped values, or memory leaks. Tests are:
 * - Stress Tests:
 *    - Basic Concurrency Check - identical, constant rates of read & write
 *    - Backpressure - high-frequency producer, slow consumer
 *    - Negative backpressure - slow producer, high-frequency consumer
 *    - Long-running wraparound 
 *    - Jittery producer and consumer 
 *    - Multiple producers and consumers
 * - Edge Test:
 *    - Mutex starvation / deadlock detection
 * - Pipeline Test 
 *    - 3 ring buffers in a pipeline with jitter at each stage
 * Author: Catherine Bernaciak PhD
 * Date: April 2025
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ring_buffer.h"
#include "test_helpers.h"

//#define NUM_WRITES 10000
#define NUM_WRITES 6 
//#define BUFFER_CAPACITY 1000
#define BUFFER_CAPACITY 4

// arguments for prod/consumer thread functions
typedef struct {
   ring_buffer *rb;
   ring_buffer *next_rb;  // optional, used in pipelining
   float *written_vals;
   float *read_vals;
   int read_rate;
   int write_rate;
   bool jitter;          // random write and read rates
} thread_args;

// bundled arguments for worker thread functions 
typedef struct{
   thread_args *input_stage;
   thread_args *output_stage;
   int thread_id;
} worker_args;

/*
 * void *producer_thread()
 * Continously writes NUM_WRITES random values to the 
 * ring buffer on a separate thread. Only used for writes to a 
 * a single buffer, does not perform reads at all. Stores the
 * values written in a separate array for later comparison.
 * Sleep time between writes is variable. Allows for jitter.
 * 
 * input void pointer
 * returns void pointer
*/
void *producer_thread(void *arg){
   //printf("producer thread ==========\n"); 
   // RHS = cast input void *arg to a thread_args* type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   int write_count = 0;

   // write continuously on this thread
   while(write_count < NUM_WRITES){
      //printf("write_count == %d\n", write_count);
      float val = (float)(rand() % 1000);
      if (ring_buffer_write(args->rb, val)){
         //printf("value written in producer thread: %f\n", val);
         args->written_vals[write_count] = val;
         write_count++;
      } 
      if (!args->jitter){
         usleep(args->write_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if write_rate = 9,000
         usleep(1000 + arc4random_uniform(args->write_rate)); 
      }
   }
   //printf("producer thread finished for this stage...\n");
   return NULL;
}

/*
 * void *consumer_thread()
 * Continously reads NUM_WRITES random values from the ring 
 * buffer on a separate thread. Only used for reads from a
 * single buffer, does not perform writes at all. Stores the
 * values read in a separate array for later comparison. 
 * Sleep time between reads is variable.  Allows for jitter. 
 *
 * input void pointer
 * returns void pointer
*/
void *consumer_thread(void *arg){

   //printf("consumer thread ==========\n"); 
   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // thread_args struct
   thread_args *args = (thread_args *)arg;
   float read_val;
   int read_count = 0;

   // perform reads on consumer thread
   while(read_count < NUM_WRITES){
      if (ring_buffer_read(args->rb, &read_val)){
         //printf("value read in consumer thread: %f\n", read_val);
         args->read_vals[read_count] = read_val;
         read_count++;
      }
      if (!args->jitter){
         usleep(args->read_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if read_rate = 9,000
         usleep(1000 + arc4random_uniform(args->read_rate)); 
      }
   }
   //printf("consumer thread finished for this stage...\n");
   return NULL;
}

/*
 * void *worker_thread()
 * Sequentially reads and writes NUM_WRITES random values from a 
 * ring buffer on a separate thread. Used for buffers intermediate 
 * in the pipeline. Stores the values read and written in separate
 * arrays for later comparison. Sleep time between writes and reads 
 * is variable. Allows for separate read and write jitter. 
 *
 * input void pointer
 * returns void pointer
*/
void *worker_thread(void *arg){
   // RHS = cast input void *arg to a thread_args * type
   // LHS = declare new variable *args that's a pointer to a 
   // worker_args struct
   worker_args *args = (worker_args *)arg;
   int tid = args->thread_id;
   //printf("worker thread %d ==========\n", tid ); 
   thread_args *in = args->input_stage;
   thread_args *out = args->output_stage;
   float read_val;
   int did_read = 0;
   int did_write = 0;
   int read_count = 0;
   int write_count = 0;
   int rw_count = 0;
   // one loop for reading and sequential writing
   while(rw_count < NUM_WRITES){
      // perform read
      if (ring_buffer_read(in->rb, &read_val)){
         //printf("value read in worker thread %d: %f\n", tid, read_val);
         in->read_vals[read_count] = read_val;
         did_read = 1;
      }
      if (!in->jitter){
         usleep(in->read_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if read_rate = 9,000
         usleep(1000 + arc4random_uniform(in->read_rate)); 
      }

      // perform write
      if (ring_buffer_write(out->rb, read_val)){
         //printf("value written in worker thread %d: %f\n", tid, read_val);
         out->written_vals[write_count] = read_val;
         did_write = 1;
      } 
      if (!out->jitter){
         usleep(out->write_rate); 
      } else {
         // realistic jitter from 1 - 10 ms if write_rate = 9,000
         usleep(1000 + arc4random_uniform(out->write_rate)); 
      }
      if(did_read && did_write){
         rw_count++;
      }
   }
   //printf("worker thread %d finished for this stage...\n", tid);
   return NULL;
}

/*
 * void thread_pressure()
 * This performs a set of stress tests for the concurrent ring buffer.
 * It allows for constant rw (basic), faster writes than reads
 * (backpressure), faster reads than writes (negative backpressure).  
 * Rate is set by two inputs, which, in the case of no jitter, represent
 * the integer number of microseconds delay between writes and reads. In 
 * the case of jitter, the inputs are 1000 us less than the max delay.
 * Ring buffer is allocated and destroyed within this function. There is 
 * a variable amount of wraparound to allow for long-running wraparound
 * (small capacity, many writes). 
 *
 * inputs int write_rate, int read_rate, int capacity, bool jitter
 * returns void
*/
void thread_pressure(int write_rate, int read_rate, int capacity, bool jitter){
   
   if(jitter) {
      printf("[TEST] Random Jitter ... \n");
   } else {
      if (capacity == BUFFER_CAPACITY){
         if (write_rate == read_rate){
            printf("[TEST] Basic Concurrency ... \n");
         } else if (write_rate > read_rate) {
            printf("[TEST] Concurrent Negative Backpressure ... \n");
         } else if (read_rate > write_rate){
            printf("[TEST] Concurrent Backpressure ... \n");
         }
      } else {
         printf("[TEST] Long running wraparound ... \n");
      }
   }

   // setup ring buffer and read/write comparison arrays 
   // inside of a thread_args struct
   thread_args *thargs = malloc(sizeof(thread_args)); 
   thargs->rb = malloc(sizeof(ring_buffer));
   thargs->written_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->read_vals = malloc(sizeof(float) * NUM_WRITES);
   thargs->write_rate = write_rate; // # us between writes
   thargs->read_rate = read_rate;   // # us between reads
   thargs->jitter = jitter;         // jitter Y/N

   // some simple tests
   assert(thargs != NULL);
   assert(thargs->rb != NULL);
   assert(thargs->written_vals != NULL);
   assert(thargs->read_vals != NULL);
   assert(ring_buffer_init(thargs->rb, capacity));

   // pthread_t is an opaque type, holds thread ID's assigned by OS
   pthread_t prod, cons;

   // start first thread
   pthread_create(&prod, NULL, producer_thread, thargs);
   // return to main thread and begin executing this one
   pthread_create(&cons, NULL, consumer_thread, thargs);

   // tell main thread to wait until these threads finish before continuing
   pthread_join(prod, NULL);
   pthread_join(cons, NULL);

   // test that all values written and read are the same - no loss
   for (int i = 0; i < NUM_WRITES; i++){
     //printf("i ====================== %d\n", i);
     //printf("written_vals[i] = %f\n", thargs->written_vals[i]);
     //printf("read_vals[i] = %f\n", thargs->read_vals[i]);
     assert(thargs->read_vals[i] == thargs->written_vals[i]);
   }

   // clean up the heap
   free(thargs->written_vals);
   free(thargs->read_vals);
   SAFE_DESTROY(thargs->rb);
   free(thargs);
   printf("OK\n");
}
/*
 * void thread_pipeline_stress()
 * This tests three ring buffers in a pipeline, passing data
 * producer -> [rb1] -> stage 1 -> [rb2] -> stage 2 -> [rb3] -> consumer
 * It currently does not allow for pressure tests, but rather has the same
 * write and read delay for each consumer and producer thread. It can 
 * allow for jitter. 
 * Ring buffers are allocated and destroyed within this function. There is 
 * a fixed amount of wraparound, depending on buffer_capacity. Each buffer
 * has same capacity.
 *
 * inputs int num_stages, int buffer_capacity
 * returns void
*/
void thread_pipeline_stress(int num_stages, int buffer_capacity){
   printf("[TEST] Modular Pipeline with Concurrency ...\n");
     
   thread_args *stages = malloc(sizeof(thread_args) * num_stages);
   assert(stages != NULL);

   for (int i = 0; i < num_stages; i++){
      // allocate & initialize one ring buffer per pipeline stage 
      stages[i].rb = malloc(sizeof(ring_buffer));
      assert(stages[i].rb != NULL);
      assert(ring_buffer_init(stages[i].rb, buffer_capacity));
      stages[i].written_vals = malloc(sizeof(float) * NUM_WRITES);
      stages[i].read_vals = malloc(sizeof(float) * NUM_WRITES); 
      stages[i].write_rate = 100;
      stages[i].read_rate = 100;
      stages[i].jitter = false;
   }
   // connect each rb's output to the next rb's input where applicable
   for (int i = 0; i < num_stages - 1; i++){
      stages[i].next_rb = stages[i+1].rb;
      assert(stages[i].next_rb != NULL);
   }
   stages[num_stages - 1].next_rb = NULL; // last stage

   // create threads and run functions
   // ... initial producer and final consumer
   pthread_t prod, cons; 
   pthread_create(&prod, NULL, producer_thread, &stages[0]);
   usleep(1000);
   // ... intermediate worker threads
   pthread_t *workers = malloc(sizeof(pthread_t) * (num_stages - 1));
   assert(workers != NULL);
   worker_args *wargs = malloc(sizeof(worker_args) * (num_stages - 1));
   assert(wargs != NULL);
   for (int i = 0; i < num_stages - 1; i++){
      wargs[i].input_stage = &stages[i];
      wargs[i].output_stage = &stages[i+1];
      wargs[i].thread_id = i;
      pthread_create(&workers[i], NULL, worker_thread, &wargs[i]);
      usleep(1000);
   }

   pthread_create(&cons, NULL, consumer_thread, &stages[num_stages-1]);
   // tell main thread to wait until these threads finish before continuing
   pthread_join(prod, NULL);
   pthread_join(cons, NULL);
   for (int i = 0; i < num_stages - 1; i++){
      pthread_join(workers[i], NULL);
   }

   // test values written to rb1 equal the values read from rb3 - no loss
   for (int i = 0; i < NUM_WRITES; i++){
     printf("i ====================== %d\n", i);
     printf("written_vals[i] = %f\n", stages[0].written_vals[i]);
     printf("read_vals[i] = %f\n", stages[num_stages-1].read_vals[i]);
     assert(stages[0].written_vals[i] == stages[num_stages-1].read_vals[i]);
   }

   // cleanup the heap
   for (int i = 0; i < num_stages; i++){
      free(stages[i].written_vals);
      free(stages[i].read_vals);
      SAFE_DESTROY(stages[i].rb);
   }
   free(workers);
   free(wargs);
   free(stages);

   printf("OK\n");

}

int main(){
   // Basic Concurrency - even write & read, no jitter
//   thread_pressure(100, 100, BUFFER_CAPACITY, 0); 
   // Backpressure - faster write than read, no jitter
//   thread_pressure(50, 100, BUFFER_CAPACITY, 0);  
   // Negative Backpressure - faster read than write, no jitter
//   thread_pressure(100, 50, BUFFER_CAPACITY, 0);  
   // Long running wrapaaround - even write, no jitter
//   thread_pressure(100, 100, 50, 0);              
   // random jitter btwn ~1-30 ms for write and read 
   // pass in max delay rate for write & read
//   thread_pressure(9000, 9000, BUFFER_CAPACITY, 1);              
   // test basic pipelining, random delays for each read and write stage
   thread_pipeline_stress(3, BUFFER_CAPACITY);
   return 0;
}
