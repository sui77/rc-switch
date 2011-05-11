#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch(3);

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0, output);
}

void loop() {

}

void output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw) {

  if (decimal == 0) {
    
    Serial.print("Unknown encoding. Raw data: ");
    for (int i=1; i< length*2; i++) {
      Serial.print(raw[i]);
      Serial.print(",");
    }
    Serial.println();
    
  } else {
    
    Serial.print("Code: ");
    Serial.print(decimal);
    Serial.print(" (");
    Serial.print( length );
    Serial.print("Bit) Binary: ");
    Serial.print( dec2binWzerofill(decimal, length) );
    Serial.print(" Delay: ");
    Serial.print(delay);
    Serial.println(" microseconds");
    
  }

}



static char * dec2binWzerofill(unsigned long Dec, unsigned int length){
 char bin[50];
 int pos = 0;
 while(Dec > 0){
  if(Dec % 2 == 0){
   bin[pos] = '0';
  }else{
   bin[pos] = '1';
  }
  Dec = floor(Dec/2);
  pos++;
 }

 char bin2[50];
 int i2=0;
 for (int i = 0; i<length-pos; i++) {
   bin2[i2++] = '0';
 }
 for (int i = pos-1; i>=0; i--) {
   bin2[i2++] = bin[i];   
 }
 bin2[i2] = '\0';

 return bin2;
}