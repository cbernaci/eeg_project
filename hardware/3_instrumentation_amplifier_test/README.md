# Instrumentation Amplifier 

The next circuit to test is the  instrumentation amplifier design which is borrowed from [this writeup](../../documentation/Design_of_Instrumentation_amplifier.pdf)
and reproduced below. 

<img src="./instr_amp_schematic.png" title="Instrumentation Amplifier Circuit Schematic." width="600"/>

I [derived](./voltage_gain_derivation.pdf) the gain of this amplifier and verified that it matched was found in the document above. If all resistors except
RG have the same resitance R, the gain becomes

$V_\text{out} = (V_1 - V_2)\left(1 + 2\frac{R}{R_G}\right)$


