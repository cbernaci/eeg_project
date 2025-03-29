
# 🧠 About eeg_project

This project implements a modular real-time electroencephalogram (EEG) signal acquisition and 
processing system in C, designed to communicate with a microcontroller (e.g., Arduino) serially 
and perform signal processing and visualization on a MacBook. The goal is to understand neurofeedback
systems by building my own. This requires first an EEG system, and second a feedback system to
augment visual data based on brain frequencies. The goal is to teach the brain to balance itself in the
time and frequency domains to potentially eliminate unwanted cognitive, emotional, and neurophysical 
symptoms. 


This phase of the project is just the EEG. The input to this system is a serial stream of digital voltage
readings from the brain acquired by way of electrodes placed on different points of the scalp, and the output
is real-time waveform data that has been processed with FFT's to extract frequency and time domain information.
This output can then be fed to an audio-visual system that is continuously alters the sound and video to alert
the human audio-visual system of perceived deviations from normal behavior.  The videos will be open-source  
and changing their display will require help from other experts, and is saved for another project.

This EEG project contains code for:
- reading, amplification, filtering, and digitization of voltages from scalp using electrodes, breadboard, and microcontroller (firmware)
   - voltage readings are obtained using electrodes on scalp
   - amplification and filtering of voltages is performed in analog domain on breadboard using op-amps and basic circuit components
   - analog voltage signals are fed to a microcontroller's ADC and digitized (preliminarily using Arduino Uno)
   - digital voltage reading passed through USB port to Macbook
- serial reading of digital data from microcontroller to Macbook M3 - uses a multi-threaded ring buffer data structure 
- digital signal processing of signals on Macbook M3
- plotting and visualization of signals in Metal and ImGui based GUI

This project is designed to run on a Macbook M3 Pro and so I am using Apple Developer tools
to get the most out of the M3 Pro chip. 

---

## 📦 Project Structure
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
## Running Application
For now, I am still building this and am working on the tests for the ring buffer. To run existing tests, go to /tests and type `make tests ; ./build/unit_test_ring_buffer`

### Author: Catherine Bernaciak PhD











