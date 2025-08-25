# Instrumentation Amplifier 

The next circuit to test is the  instrumentation amplifier design which is borrowed from [this writeup](../../documentation/Design_of_Instrumentation_amplifier.pdf)
and reproduced below. 

<img src="./instr_amp_schematic.png" title="Instrumentation Amplifier Circuit Schematic." width="600"/>

I [derived](./voltage_gain_derivation.pdf) the gain of this amplifier and verified that it matched was found in the document above. If all resistors except
$R_G$ have the same resitance R, the gain becomes

$V_\text{out} = (V_1 - V_2)\left(1 + 2\frac{R}{R_G}\right)$

I chose $R=20k\Omega$ and $R=100\Omega$ which will yield a gain of about 400 times.

## Circuit
I wired up the circuit on a breadboard using three LM741 op-amps. These are quite old, cheap op-amps but they are all that I had. 
These require a large DC voltage for power so this instrumetnation amplifier is not suitable for plugging into the Arduino and then into my MacBook. I am also
not willing to connect sensors from my scalp to this circuit. So, to measure the output voltage I used an oscilloscope and to simulate EEG signals I used an 
Arbitrary Waveform Generator to create low voltage oscillating signals as input voltages $V_1$ and $V_2$.  

<img src="./breadboard_farthest_view.jpg" title="Instrumentation Amplifier Full Setup." width="800"/>

The DC supply is unable to output a negative voltage so to create a $-15V$ I created an artificial ground by connecting CH1+ to CH2- and wiring those both 
to the ground of the breadboard. CH1- is wired to the negative voltage supply (pin 4) of each op-amp and CH2+ is wired to the positive voltage supply (pin 7)
of each op-amp via the red wires which are connected to two of the power rails on the breadboard..  

<img src="./dc_wiring_schematic.png" title="DC Voltage Supply Setup." width="600"/>

A close-up of the circuit wiring is shown below. 

<img src="./breadboard_closest_view.jpg" title="Breadboard Close-up." width="600"/>

The white wires are coming from CH1 and CH2 of the AWGs which are stand-ins for real EEG signals. Since this circuit operates differentially, 
I set CH1 and CH2 to sine waves with amplitudes, 

* 2mVpp, 1kHZ
* 6mVpp, 1kHz

<img src="./awg_channel12_view.jpg" title="AWG Settings." width="600"/>

With a gain of about 400, the waveform measured by the ocilloscope should have a voltage of approximately 4mVpp * 400 = 1.6Vpp.  I observed a 
voltage of about 1.7Vpp on the oscilloscope. See right-hand side box on screen below:

<img src="./oscilloscope_msnt.jpg" title="AWG Settings." width="600"/>

While this instrumentation amplifier does behave as expected, these op-amps require such a high voltage that it is not safe for connecting to either
human or PC.  I also did not simulate frequencies that are within the range of brainwaves. Two things then need to be done:

1. re-run with lower frequency
2. get op-amps that can use a DC supply voltage that can be supplied by a battery. 




