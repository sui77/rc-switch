/*
  Example for different sending methods
  
  http://code.google.com/p/rc-switch/
*/

#include <RCSwitch.h>

// Transmitter is connected to Arduino Pin #10
RCSwitch mySwitch = RCSwitch(3);

void setup() {

  Serial.begin(9600);
  
  /* See Example: TypeA_WithDIPSwitches */
  mySwitch.switchOn("11111", 4); 
  delay(1000);  
  mySwitch.switchOff("11111", 4);
  delay(1000);

  /* Same switch as above, but using decimal code */
  mySwitch.send(5393, 24);
  delay(1000);  
  mySwitch.send(5396, 24);
  delay(1000);  

  /* Same switch as above, but using binary code */
  mySwitch.send("000000000001010100010001");
  delay(1000);  
  mySwitch.send("000000000001010100010100");
  delay(1000);

  /* Same switch as above, but tri-state code */ 
  mySwitch.sendTriState("00000FFF0F0F");
  delay(1000);  
  mySwitch.sendTriState("00000FFF0FF0");
  delay(1000);

}

void loop() {
}