/*
  Example for outlets which are configured with two rotary/sliding switches.
  
  http://code.google.com/p/rc-switch/
*/

#include <RCSwitch.h>

// Transmitter is connected to Arduino Pin #10
RCSwitch mySwitch = RCSwitch(10);

void setup() {
}

void loop() {
  // Switch on:
  // The first parameter represents the setting of the first rotary switch. 
  // In this example it's switched to "1" or "A" or "I". 
  // 
  // The second parameter represents the setting of the second rotary switch. 
  // In this example it's switched to "4" or "D" or "IV". 
  mySwitch.switchOn(1, 4);

  // Wait a second
  delay(1000);
  
  // Switch off
  mySwitch.switchOff(1, 4);
  
  // Wait another second
  delay(1000);
}
