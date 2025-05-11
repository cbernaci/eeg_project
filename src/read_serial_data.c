#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>
#include "eeg_config.h"
#include "ring_buffer.h"

#define SERIAL_PORT "/dev/cu.usbmodem11301"
#define BAUD_RATE B115200
#define FLOAT_SIZE sizeof(float)

extern float display_buffer[NUM_POINTS];

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

void *serial_reader(void *arg){
   while (1) {

      int fd = *(int *)arg;  // cast void* back to int* 
      char buffer[FLOAT_SIZE];
      int bytes_read;
      //int test_counter = 0;  
      //int test_max_counter = 100;  

      // read 4 bytes of binary data from serial port
      bytes_read = read(fd, buffer, FLOAT_SIZE);
      if(bytes_read == FLOAT_SIZE){ 
         //test_counter++;
         float voltage;
         memcpy(&voltage, buffer, FLOAT_SIZE);
         printf("Voltage: %.2f\n", voltage);  // print incoming data
      }
      //if(test_counter==test_max_counter) break;
   }
   return NULL;
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

int main_42(){

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
      return 1;
   }

   // the serial port is denoted by fd and is configured with this call
   setup_serial(fd);

   // create separate thread for serial reading
   pthread_t thread_id; // identifer to reference the thread
  
   if(pthread_create(&thread_id, NULL, serial_reader, &fd) != 0){
      perror("Failed to create thread for serial reading");
      return 1;
   }

   while (1) {
      sleep(1);
   }
    

    
   close(fd);
   return 0;
}
