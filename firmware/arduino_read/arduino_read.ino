// a sketch to generate a PWM square wave voltage (0V - 5V) from pin 9
// then read voltage from analog pin A0 from Macbook
// this is just an example to test serial data input to the Mac
// I will use this stream to test a ring buffer data structure
// A0 reads input voltage (square wave)
// Author: Catherine Bernaciak, PhD
// Date: Mar 2025

const int pwmPin = 9;         // PWM output pin, 490 Hz default
const int dutyCycle = 127;    // duty cycle of 50% (0->255)
const int srcPin = A0; 

// this function executed only once when arduino starts or resets
// must be here, even if empty
void setup() {

   // start serial communication at 115200 baud rate
   Serial.begin(115200); 
   // configure pin 9 as output
   pinMode(pwmPin, OUTPUT);    
   // generate 50% duty cycle square wave from pin 9
   analogWrite(pwmPin, dutyCycle); 
}
// every sketch must have loop() function
// runs continuously after setup() has finished
void loop() {

   // read the voltage from A0
   int sensorValue1 = analogRead(srcPin);
   // convert the sensor reading to voltage value
   // divide by 1023 bc arduino has 10-bit ADC (2^10 = 1024) starting at value 0
   float voltage1 = sensorValue1*(5.0/1023.0);
   // send voltage reading to the serial monitor
   //Serial.println(voltage1); // sends as string terminated with \r\n
   Serial.write((byte*)&voltage1, sizeof(voltage1));
   delayMicroseconds(5);
   Serial.flush();
}
