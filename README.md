
# 🧠 About eeg_project

This project implements a real-time electroencephalogram (EEG) signal acquisition and 
processing system in C.  The goal of this project is to gain some understanding of the
time and frequency domains of the brain at the network level. 

The input to this system is a serial stream of digital voltage readings from the brain
acquired by way of electrodes placed on different points of the scalp. The output is
real-time waveform data that has been processed with FFT's to extract frequency and time
domain information.

This EEG project contains code for:
- microcontroller firmware (Arduino/C++)
   - voltage readings are obtained using electrodes on scalp
   - amplification and filtering is performed in analog domain using op-amps and basic circuit components
   - analog voltage signals are fed to a microcontroller's ADC and digitized (preliminarily using Arduino Uno)
   - digital voltage reading passed through USB port to Macbook
- serial reading of digital data (C)
   - serial port of Macbook M3 using a multi-threaded ring buffer data structure 
- digital signal processing of signals on Macbook M3 (C, Apple Accelerate vDSP)
   - preprocessing (including filtering and noise removal)
   - feature extraction by computing FFT and power spectral density for better visualization
- GUI for plotting and visualization of signals (C, Apple Metal, ImGui)
   - separate visualization thread using GPU acceleration 

This project is designed to run on a Macbook M3 Pro and so I am using Apple Developer tools
to get the most out of the M3 Pro chip. 

# 📦 Project Structure
```
eeg_project/ 
├── src/       # C source files for EEG application
├── include/   # header files for function declarations
├── tests/     # unit, edge case, and stress tests 
├── build/     # compiled binaries
├── firmware/  # Arduino code for signal acquisition
├── Makefile   # build tests or application
└── README.md 
```

# 🤔 Running Tests 
The tests that are currently implemented are unit tests for the ring buffer data structure. 
To run existing tests:

`make tests ; ./build/unit_test_ring_buffer`

# 🚀 Running Application
Application is not ready - I am still in the testing and construction phase

# ✨ Follow-up Work: Neurofeedback
The EEG project will be the first part of a neurofeedback system that will augment visual
data based on brain frequencies. Specifically, the output of the EEG project will be fed
to a neurofeedback system that will take detected deviations from optimal time and frequency
domain behavior and use it to continously augment audo-visual data in real-time in a way that
makes the video slow down, lose color, or be changed in subtle ways that are subconsciously 
not desired by the brain. 

The human audio-visual system detects the alterations and learns to 
change it's connectivity and hence frequency and time domain output in a more optimal way such 
that the audio-visual data it is collecting becomes more pleasant to view - colors brighten,
movement quickens, music gets more appealing, etc.  The goal is to teach the brain to balance
itself in the time and frequency domains to potentially eliminate unwanted cognitive, emotional, 
and neurophysical symptoms.  The videos will be open-source and changing their sound and display
will require help from other experts, and is saved for another project.

### Author: Catherine Bernaciak PhD











