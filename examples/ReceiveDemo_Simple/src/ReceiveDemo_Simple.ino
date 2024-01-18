/*
  Simple example for receiving
  
  https://github.com/sui77/rc-switch/
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
    
    Serial.print("Received ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );

    mySwitch.resetAvailable();
  }
}
