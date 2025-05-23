
// include guard
#ifndef READ_SERIAL_DATA_H
#define READ_SERIAL_DATA_H

#include "ring_buffer.h"
/*
 * @brief Fills a ring buffer with sine wave data
 *
 * The function fills a ring buffer with data of sine wave moving 
 * in the positive x direction. It's used as a placeholder for
 * testing the eeg app before real brainwave data is available
 * @param rb Pointer to a ring buffer instance
 * @return void
*/
void sine_data_stream(ring_buffer *rb);

/*
 * @brief Fills a ring buffer with real EEG data
 *
 * The function fills a ring buffer with data from a single
 * channel on the pre-frontal cortex. Data is from physionet.org
 * from the dataset entitled 'Motion Artifact Contaminated fNIRS 
 * and EEG Data', Trail1.csv
 *
 * @param rb Pointer to a ring buffer instance
 * @return void
*/
void read_physionet_data(ring_buffer *rb);

/*
 * @brief Fills a ring buffer with serial port data
 *
 * The function fills a ring buffer with data from the USB 
 * port of the Macbook. 
 *
 * @param rb Pointer to a ring buffer instance
 * @return void
*/
void read_serial_data(ring_buffer *rb);

#endif
