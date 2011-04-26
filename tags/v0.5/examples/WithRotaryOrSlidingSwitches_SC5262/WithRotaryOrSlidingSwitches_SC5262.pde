#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch(10);  // Sender is connected to Pin #10

void setup() {
}

void loop() {
  mySwitch.switchOn(1, 1);         // Switch 1st socket on
  delay(1000);
  mySwitch.switchOff(1, 1);        // Switch 1st socket off
  delay(1000);
}
