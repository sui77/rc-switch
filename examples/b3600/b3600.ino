/*
  Example for brennenstuhl 3600 sockets
  
  http://code.google.com/p/rc-switch/
  
  Need help? http://forum.ardumote.com

These codes were transcribed from a scope screen.
I found more here http://pastebin.com/RgQ4VCyw
But only the on codes worked, the off codes didn't.
*/

char * on_codes[] = {
    "101001000110001010101100",
    "101011011010011000000101",
    "101000001100100101111110",
};

char * off_codes[] = {
    "101001110111110010111100",
    "101000010101101001000101",
    "101010111011000001011110",
};

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {

  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(A4);
  mySwitch.setProtocol(4);
}


void loop() {

  // Switch on:
  mySwitch.send(on_codes[2]);
  delay(2000);
  // Switch off:
  mySwitch.send(off_codes[2]);
  delay(2000);
}

