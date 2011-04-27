/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.
  
  Project home: http://code.google.com/p/rc-switch/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "RCSwitch.h"
 
/**
 * Constructor
 *
 * @param nPin    Arduino Pin to which the sender is connected to
 */
RCSwitch::RCSwitch(int nPin) {
  this->nPin = nPin;
  this->nDelay = 469;
  pinMode(nPin, OUTPUT);
}

/**
 * Constructor with different bit width
 *
 * @param nPin    Arduino Pin to which the sender is connected to
 * @param nDelay  1/8 Bit width in microseconds
 */
RCSwitch::RCSwitch(int nPin, int nDelay) {
  this->nPin = nPin;
  this->nDelay = nDelay;
  pinMode(nPin, OUTPUT);
}

/**
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOn(int nAddressCode, int nChannelCode) {
  this->send( this->getCodeWord(nAddressCode, nChannelCode, true) );
}

/**
 * Switch a remote switch off (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOff(int nAddressCode, int nChannelCode) {
  this->send( this->getCodeWord(nAddressCode, nChannelCode, false) );
}

/**
 * Switch a remote switch off (Type B with two rotary/sliding switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOn(String sGroup, int nChannel) {
  this->send( this->getCodeWord2(sGroup, nChannel, true) );
}

/**
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOff(String sGroup, int nChannel) {
  this->send( this->getCodeWord2(sGroup, nChannel, false) );
}

/**
 * Returns a String[13], representing the Code Word to be send. 
 * A Code Word consists of 9 address bits, 3 data bits and one sync bit but in our case only the first 8 address bits and the last 2 data bits were used.
 * A Code Bit can have 4 different states: "F" (floating), "0" (low), "1" (high), "S" (synchronous bit)
 *
 * +-------------------------------+--------------------------------+-----------------------------------------+-----------------------------------------+----------------------+------------+
 * | 4 bits address (switch group) | 4 bits address (switch number) | 1 bit address (not used, so never mind) | 1 bit address (not used, so never mind) | 2 data bits (on|off) | 1 sync bit |
 * | 1=0FFF 2=F0FF 3=FF0F 4=FFF0   | 1=0FFF 2=F0FF 3=FF0F 4=FFF0    | F                                       | F                                       | on=FF off=F0         | S          |
 * +-------------------------------+--------------------------------+-----------------------------------------+-----------------------------------------+----------------------+------------+
 * 
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 * @param bStatus       Wether to switch on (true) or off (false)
 * 
 * @return String[13]
 */
String RCSwitch::getCodeWord(int nAddressCode, int nChannelCode, boolean bStatus) {
   String code[5] = { "FFFF", "0FFF", "F0FF", "FF0F", "FFF0" };

   if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4) {
    return "";
   }

   return code[nAddressCode] + code[nChannelCode] + "FF" + (bStatus==true?"FF":"F0") + "S";
}

/**
 * Like getCodeWord  (Type A)
 */
String RCSwitch::getCodeWord2(String sGroup, int nChannelCode, boolean bStatus) {
	String code[6] = { "FFFFF", "0FFFF", "F0FFF", "FF0FF", "FFF0F", "FFFF0" };

	if (sGroup.length() != 5 || nChannelCode < 1 || nChannelCode > 5) {
		return "";
	}
	
	String sAddressCode = "";
	for (int i = 0; i<5; i++) {
		if (sGroup[i] == '0') {
			sAddressCode += "F";
		} else {
			sAddressCode += "0";
		}
	}
	
	return sAddressCode + code[nChannelCode] + (bStatus==true?"0F":"F0") + "S";
}


/**
 * Sends a Code Word 
 * @param sCodeWord   /^[10FS]{13}$/  -> see getCodeWord
 */
void RCSwitch::send(String sCodeWord) {
  
  for (int nRepeat=0; nRepeat<10; nRepeat++) {
    for(int i=0; i<=12; i++) {
      switch(sCodeWord[i]) {
        case '0':
          this->send0();
        break;
        case 'F':
          this->sendF();
        break;
        case '1':
          this->send1();
        break;
        case 'S':
          this->sendSync();
        break;
      }
    }
  }
}

/**
 * Sends a "0" Bit
 *            _     _
 * Waveform: | |___| |___
 */
void RCSwitch::send0() {
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 1);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 3);
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 1);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 3);
}

/**
 * Sends a "1" Bit
 *            ___   ___
 * Waveform: |   |_|   |_
 */
void RCSwitch::send1() {
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 3);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 1);
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 3);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 1);
}

/**
 * Sends a "F" Bit
 *            _     ___
 * Waveform: | |___|   |_
 */
void RCSwitch::sendF() {
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 1);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 3);
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 3);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 1);
}

/**
 * Sends a "S" Bit
 *            _ 
 * Waveform: | |_______________________________
 */
void RCSwitch::sendSync() {
  digitalWrite(this->nPin, HIGH);
  delayMicroseconds( this->nDelay * 1);
  digitalWrite(this->nPin, LOW);
  delayMicroseconds( this->nDelay * 31); 
}