/*
  Example for receiving
  
  https://github.com/sui77/rc-switch/
  
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  // for some transmitters it gives better results when you set it to true
  // when it is false library is using initial synchronization pulse to determine timing for decoding signal
  // when it is true library is using protocol definition to determine timing for decoding signal
  mySwitch.setReceiveUsingProtocolTiming(false);
  // this is for testing only - it will show timings of received data even, if it doesn't follow any protocol.
  mySwitch.setReceiveUnknownProtocol(true);
}

void loop() {
  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
