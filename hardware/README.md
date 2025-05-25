# Circuit Documentation
Here I document the circuits tested at each step while building the EEG. I wanted to test the 
SW side of things before hooking up electrodes to my scalp, so I used the Arduino Uno to generate a
square wave signal with a duty cycle of 50% to drive some circuits, which each circuit getting 
progressivly closer to the signal chain needed to handle the electrode signal. 

The order of progression for the circuits are:

1. square wave only circuit
2. low-pass filter circuit
3. pre-amp circuit
	* single op-amp
	* instrumentation amplifier
4. signal chain circuit
	* idk do i need this or is instrumentation amplifier enough?


## Square-Wave Only Circuit
The Arduino Uno is used to generate a square wave with 50% duty cycle from pin 9. Pin 9 is then 
connected directly to Pin A0 which sends the signal to the digitizer and then to the Serial Monitor. 
Check the arduino sketch /eeg_project/firmware/arduino_read_square_wave/arduino_read_square_wave.ino
to see how to generate the square wave. 

Pic of Circuit:
<img src="./square_wave_uno.png" title="Uno Wiring for Square Wave." width="300"/>

Pic of Signal:
![Visualizer_square_wave.](./square_wave.png "EEG Visualizer with Square Wave.")


