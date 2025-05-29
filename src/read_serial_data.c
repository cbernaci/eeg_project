#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include "eeg_config.h"
#include "ring_buffer.h"

#define SERIAL_PORT "/dev/cu.usbmodem11301"
#define BAUD_RATE B115200
#define FLOAT_SIZE sizeof(float)

// timing for debugging
//struct timeval tv;
//double t_start = tv.tv_sec + tv.tv_usec / 1e6;
//gettimeofday(&tv, NULL);

/*
 * The function fills a ring buffer with y-axis value of a sine wave 
 * It's used as a placeholder for testing the eeg app before real 
 * brainwave data is available.
*/
void sine_data_stream(ring_buffer *rb){
   float phase = 0.0f;
   while(1){
      float sample = 0.5f * sinf(6.0f * phase);
      ring_buffer_write(rb, sample);
      // fill the display buffer (uncomment if using)
      //memmove(display_buffer, display_buffer + 1, (NUM_POINTS - 1)*sizeof(float));
      //display_buffer[NUM_POINTS - 1] = sample;
      phase += -0.02f;
      //usleep(5000);    // ~200Hz data stream
      //usleep(1000);    // ~1kHz data stream
      //usleep(500);     // ~2kHz data stream
      //usleep(250);     // ~4kHz data stream
      usleep(200);       // ~5kHz data stream
      //usleep(100);     // ~10kHz data stream
   }
}

/*
 * A function writes to a ring buffer data from a physionet.org dataset
 * sampled at 2048 Hz for about 1 minute. It therefore contains about 140K
 * readings which come from two different point on the pre-frontal cortex.
 * It doesn't say which points.  
 *
*/
void read_physionet_data(ring_buffer *rb){

   // 1. open csv file
   char *filename = "data/EEG-csv/Trial1.csv"; // relative to makefile
   FILE *file = fopen(filename, "r");
   if (!file){
      printf("Error opening file: %s\n", filename);
      return;
   }
   // 2. read one line at a time
   char *line = NULL; // pointer to line buffer
   size_t len = 0;   // getline will allocate space if we don't know ahead
   ssize_t read;     // the line we read
   while ((read = getline(&line, &len, file)) != -1){
      // 3. extract only 2nd column and write to ring buffer
      char *token;
      float value;
      int count = 0;
      // split line on commas, tokens is an array of char's between commas
      token = strtok(line, ",");
      if (token) { // if there is a first token
         token = strtok(NULL, ",");  // grab second token if there is one
         if (token) { 
            count++;
            value = atof(token);
            // assuming min is 8000, max is 10,000, determined from
            // visually inspecting file but there are a few values outside
            // of this range but this should be the vast majority. 
            float scaled_value = (value - 8000)/1000 - 1;
            if( (value < 8000) | (value > 10000)){
               printf("===============\n");
               printf("value orig = %f\n", value);
               printf("value scaled = %f\n", scaled_value);
               printf("iteration = %d\n", count);
            }
            ring_buffer_write(rb, scaled_value);
//            if (count++ == 200) {exit(1);}
//            printf("2nd token is %d: \n", value);
         }
      }
      usleep(500);     // ~2kHz data stream
//      exit(1);
   }
}

/*
 * This function reads the serial stream and writes 
 * to a ring_buffer
 */
void serial_reader(int fd_in, ring_buffer *rb_in){

      char buffer[FLOAT_SIZE];
      int test_counter = 0;  
      time_t last_time = time(NULL);
      //int test_max_counter = 100;  

      while (keep_running) {
         size_t total_bytes_read = 0;

         // accumulate 4 bytes before writing a voltage value to ring buffer
         while (total_bytes_read < FLOAT_SIZE){
            // read returns number of bytes read from the file descriptor fd_in
            int n = read(fd_in, buffer + total_bytes_read, FLOAT_SIZE - total_bytes_read);

            if (n > 0){ // have either read 1,2,3 bytes
               total_bytes_read += n;
            } else if (n == 0){
               // EOF or no data - skip this read
               continue;
            } else {
               // should have read something
               perror("Serial read error");
               break;
            }
         }

         // we've read 4 bytes, now write to ring_buffer
         if(total_bytes_read == FLOAT_SIZE){ 
            float voltage;
            memcpy(&voltage, buffer, FLOAT_SIZE);
//            printf("[SERIAL] voltage being written: %.6f\n", voltage);  // print incoming data
            ring_buffer_write(rb_in, voltage);
            test_counter++;
            //usleep(500);       // 2.5kHz data stream
         }
         //if(test_counter==test_max_counter) break;

         //measure how fast data is being read from serial port
         // Note: this gives about 2100 Hz (taken May 27 2025)
        /* 
         time_t now = time(NULL);  
         // condition is true only once per second, so this triggers a count
         // of the number of reads per second bc test_counter is incremented
         // right after a read. 
         if (now > last_time){  
            printf("[SERIAL] Floats per second: %d\n", test_counter);
            test_counter = 0;
            last_time = now;
         }
        */ 
        
      }
}

void setup_serial(int fd){
   // to find all settings: find / -name 'termios.h'
   // struct to hold serial port settings
   struct termios tty;

   // retrieve current serial port settings and store in tty
   if(tcgetattr(fd, &tty) != 0) {
      perror("tcgetattr error: cannot get terminal attributes");
      exit(1);
   }

   // set baud rate 
   cfsetospeed(&tty, BAUD_RATE);
   cfsetispeed(&tty, BAUD_RATE);

   // c_cflag = 32-bit unsigned long for HW control of terminal
   // CS8 = 8-bit data, CLOCAL = ignore modem control signals
   //printf("c_cflag = %lx\n", tty.c_cflag);
   tty.c_cflag &= ~PARENB;      // disable parity bit
   //printf("c_cflag with parity disabled = %lx\n", tty.c_cflag);
   tty.c_cflag &= ~CSTOPB;      // use 1 stop bit 
   //printf("c_cflag with one stop bit = %lx\n", tty.c_cflag);
   tty.c_cflag &= ~CSIZE;       // size bits cleared
   //printf("c_cflag with size bits cleared = %lx\n", tty.c_cflag);
   tty.c_cflag |= CS8;          // set 8 data bits
   //printf("c_cflag with 8 data bits = %lx\n", tty.c_cflag);
   tty.c_cflag &= ~CCTS_OFLOW;  // disable CTS signals (only need for UART)
   //printf("c_cflag without CTS signals = 0x%lx\n", tty.c_cflag);
   tty.c_cflag &= ~CRTS_IFLOW;  // disable RTS signals (only need for UART)
   //printf("c_cflag without RTS signals = 0x%lx\n", tty.c_cflag);
   tty.c_cflag |= CREAD;        // enable receiver 
   //printf("c_cflag with receiver enabled = 0x%lx\n", tty.c_cflag);
   tty.c_cflag |= CLOCAL;       // ignore modem status lines 
   //printf("c_cflag with modem status lines disabled = 0x%lx\n", tty.c_cflag);

   // c_iflag = 32-bit unsigned long for SW input processing 
   //printf("c_iflag = %lx\n", tty.c_iflag);
   tty.c_iflag &= ~(IXON | IXOFF | IXANY);
   //printf("c_iflag after SW flow control disabled = %lx\n", tty.c_iflag);
   tty.c_iflag &= ~(INLCR | ICRNL);
   //printf("c_iflag after turning off LF and CR conversions = %lx\n", tty.c_iflag);

   // c_oflag = 32-bit unsigned long for SW output processing
   //printf("c_oflag = %lx\n", tty.c_oflag);
   tty.c_oflag &= ~OPOST;       // disable following output processing

   // c_lflag = 32-bit unsigned long local flags 
   //printf("c_lflag = %lx\n", tty.c_lflag);
   tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // raw input, no echo, no canonical mode
   
   // c_cc[] array for special control characters
   //printf("c_cc[VMIN] = %d\n", tty.c_cc[VMIN]); 
   tty.c_cc[VMIN] = 1; // minimum of 1 character to read 
   //printf("c_cc[VMIN] after change = %d\n", tty.c_cc[VMIN]); 

   //printf("c_cc[VTIME] = %d\n", tty.c_cc[VTIME]); 
   tty.c_cc[VTIME] = 10; // wait 1s before timeout, unit is 0.1 seconds 
   //printf("c_cc[VTIME] = %d\n", tty.c_cc[VTIME]); 

   tcflush(fd, TCIFLUSH); // flush buffer - discard unprocessed or unread data 

   if(tcsetattr(fd, TCSANOW, &tty) != 0) { // TCSANOW = make change immediate
      perror("tcsetattr error: cannot set terminal attributes");
      exit(1);
   }
}

void read_serial_data(ring_buffer *rb){


   // O_RDWR = open serial port for read and write
   // O_NOCTTY = don't let serial port be a controlling terminal 
   // meaning if data comes in it can't affect the program
   // O_RDWR | O_NOCTTY is a bitmask, so both of these things become true

   // fd = file descriptor, a number that identifies the open file
   // a file descriptor is a small integer that the OS uses to keep track
   // of open files, devices, or sockets in a program. 
   // fd = 0 (stdin, keyboard)
   // fd = 1 (stdout, terminal output)
   // fd = 2 (stderr, error output)
   // fd = 3 (/dev/tty.usbserial, our serial port)

   int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
   printf("fd = %d\n", fd);
   if (fd == -1) {
      perror("Error opening serial port");
   }

   setup_serial(fd);      // configure serial port fd
   while (keep_running) {
      serial_reader(fd, rb); // write serial data to ring_buffer 
   }

   close(fd);
   printf("Serial port closed.\n");
}
