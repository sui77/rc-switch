/*
  Example for Hyttevokteren outlets.

  Created by: Kåre Smith, https://github.com/ksmith3036
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void flash() {
      for (int i = 0; i < 5; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
}

void setup() {

  Serial.begin(115200);
  while (!Serial);

  // Receiver is connected to Arduino Pin #0
  int recvInterupt = digitalPinToInterrupt(0);
  Serial.print("Recv int: ");
  Serial.println(recvInterupt);

  // Transmitter is connected to Arduino Pin #7
  mySwitch.enableTransmit(7);
  mySwitch.setProtocol(1); // Hyttevokter kontakt og hyttrevokter fjernkontroll
  mySwitch.setPulseLength(250); // Hyttevokter kontakt
  //mySwitch.setPulseLength(316); // Hyttevokter remote control B002R


  pinMode(LED_BUILTIN, OUTPUT);

  delay(2000);
  Serial.println();
  Serial.print("on or off? ");
}


void loop() {

  // Switch on/off:
  // The first  parameter represents the switch number
  // The second parameter represents the familycode (a, b, c, ... f)
  
  static int lastKey = LOW;
  static bool setOn = false;

  if (Serial) {
    if (Serial.available()) {
      String key = Serial.readString();
      key.trim();
  
      setOn = key == "on";
      flash();    
      if (setOn) {
        Serial.print("set on ...");
        digitalWrite(LED_BUILTIN, HIGH);
        mySwitch.switchOn(4, 'G');
        digitalWrite(LED_BUILTIN, HIGH);
      }
      else {
        Serial.print("off ...");
        digitalWrite(LED_BUILTIN, LOW);
        mySwitch.switchOff(4, 'G');
        digitalWrite(LED_BUILTIN, LOW);
      }
      Serial.print("\non or off? ");
    }
  }
}
