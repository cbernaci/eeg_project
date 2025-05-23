// a sketch to generate a PWM square wave voltage (0V - 5V) from pin 9
// then read voltage from analog pin A0 from Macbook
// this is just an example to test serial data input to the Mac
// I will use this stream to test a ring buffer data structure
// A0 reads input voltage (square wave)
// Author: Catherine Bernaciak, PhD
// Date: Mar 2025

const int outputPin = 9;         // PWM output pin, 490 Hz default
const int inputPin = A0; 
const int sampleCount = 256;
int sineWave[sampleCount];
// this function executed only once when arduino starts or resets
// must be here, even if empty
void setup() {

   Serial.begin(115200);           // set baud rate to 115200 
   pinMode(outputPin, OUTPUT);     // configure pin 9 as output
   // initialize sine wave lookup table
   for (int i = 0; i < sampleCount; i++){
      sineWave[i] = 127 + 127*sin(2*PI*i/sampleCount);
   } 
}
// every sketch must have loop() function
// runs continuously after setup() has finished
void loop() {

   for (int i = 0; i < sampleCount; i++){
      analogWrite(outputPin, sineWave[i]);               // output sine wave from pin 9
      delayMicroseconds(100);                            // let the signal propagate
      int sensorValue = analogRead(inputPin);            // read input signal on A0
      float voltage = sensorValue*(5.0/1023.0);          // convert to 0-5V
      Serial.write((byte*)&voltage1, sizeof(voltage1));  // pass value to serial monitor to usb
      //Serial.println(voltage1); // sends as string terminated with \r\n
      //delayMicroseconds(5);
      //Serial.flush();
   }
}
