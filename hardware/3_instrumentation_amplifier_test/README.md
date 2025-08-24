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

<img src="./breadboard_farthest_view.jpg" title="Instrumentation Amplifier Full Setup." width="600"/>
