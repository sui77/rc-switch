#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch(10);  // Sender is connected to Pin #10

void setup() {
}

void loop() {
  mySwitch.switchOn("11101", 1);
  delay(1000);
  mySwitch.switchOff("11101", 1);
  delay(1000);
}
