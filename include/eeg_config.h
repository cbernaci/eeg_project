#ifndef EEG_CONFIG_H
#define EEG_CONFIG_H

#include <signal.h>

/*
 * This is for stopping the app. It's defined in int main of 
 * eeg_app.c and used in void serial_reader of read_serial_data.c
 */
extern volatile sig_atomic_t keep_running; // for stopping the app

#pragma once
#define NUM_POINTS 250

#endif
