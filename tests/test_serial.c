// Tests/mock for serial data handling

/* tests that will involve mocking serial data (simulating microcontroller input)
instead of reading real data from the Arduino, we simulate fake EEG-like data streams
inside the test and pretend i'm receiving bytes from the serial port. This can allow
testing w/o needing Arduino physically connected. This can also allow testing edge cases
like noisy data or unexpected behavior. 

Will create a mock serial read function that 
* returns predictable byte sequences
* returns corrupted or partial data
* simulates serial timeouts or disconnects

*/
