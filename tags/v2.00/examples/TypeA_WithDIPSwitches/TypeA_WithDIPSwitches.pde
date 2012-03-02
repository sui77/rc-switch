/*
  Example for outlets which are configured with a 10 pole DIP switch.
  
  http://code.google.com/p/rc-switch/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(10);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  
}

void loop() {

  // Switch on:
  // The first parameter represents the setting of the first 5 DIP switches. 
  // In this example it's ON-ON-OFF-OFF-ON.
  // 
  // The second parameter represents the setting of the last 5 DIP switches. 
  // As only one of these must be ON, it's the number of the DIP switch which 
  // is ON. In this example the last 5 DIP switches are OFF-ON-OFF-OFF-OFF.  
  mySwitch.switchOn("11001", 2);

  // Wait a second
  delay(1000);
  
  // Switch off
  mySwitch.switchOff("11001", 2);
  
  // Wait another second
  delay(1000);
  
}
