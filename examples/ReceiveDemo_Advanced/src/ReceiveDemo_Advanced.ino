/*
  Example for receiving
  
  https://github.com/sui77/rc-switch/
  
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(MONITOR_SPEED);

  delay(1000);
  Serial.print("Listening on pin ");
  Serial.println(RCSWITCH_RECIEVE_PIN);

  pinMode(RCSWITCH_RECIEVE_PIN, INPUT);
  mySwitch.enableReceive(RCSWITCH_RECIEVE_PIN);  // see platformio_shared.ini
}

void loop() {
  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
