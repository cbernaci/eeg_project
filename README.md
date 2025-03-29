<<<<<<< HEAD
Author: Catherine Bernaciak PhD
Data: Jan - April 2025 (so far)

I am building my own EEG system for neurofeedback (NFBK) purposes. 
I want to learn about brain frequencies and which patterns correlate to different symptoms
I want to create my own neurofeedback system, maybe based on free YouTube videos that i can
ammend for my own NFBK treatment. 

This project will contains just the EEG portion of the system and include:
=> reading, amplification, filtering, and digitization of voltages from scalp using electrodes, breadboard, and microcontroller
=> serial reading of digital data from microcontroller to Macbook M3
=> digital signal processing of signals on Macbook M3
=> plotting and visualization of signals in Metal and ImGui based GUI

This project is designed to run on a Macbook M3 Pro and so I am using Apple Developer tools
to get the most out of the M3 Pro chip. 

=======
# eeg_project

# 🧠 Real-Time EEG Signal Processing System (C)

This project implements a modular real-time EEG signal acquisition and processing system in C, designed to communicate with a microcontroller (e.g., Arduino) via serial and perform signal processing and visualization on a MacBook.


---

## 📦 Project Structure
```plaintext
eeg_project/ 
├── src/ # Core source code (serial, DSP, ring buffer)
├── include/ # Public header files 
├── tests/ # Unit tests (ring buffer, serial, DSP)
├── build/ # Compiled binaries 
├── firmware/ # Arduino code for EEG acquisition 
├── Makefile # Build system 
└── README.md # You're here!




