/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.
  
  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
  - Andreas Steinel / A.<lastname>(at)gmail(dot)com
  - Max Horn / max(at)quendi(dot)de
  
  Project home: https://github.com/sui77/rc-switch/

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

static const RCSwitch::Protocol PROGMEM proto[] = {
    { 350, {  1, 31 }, {  1,  3 }, {  3,  1 } },    // protocol 1
    { 650, {  1, 10 }, {  1,  2 }, {  2,  1 } },    // protocol 2
    { 100, {  1, 71 }, {  4, 11 }, {  9,  6 } },    // protocol 3
    { 380, {  1,  6 }, {  1,  3 }, {  3,  1 } },    // protocol 4
    { 500, {  6, 14 }, {  1,  2 }, {  2,  1 } },    // protocol 5
};

static const int numProto = sizeof(proto) / sizeof(proto[0]);

#if not defined( RCSwitchDisableReceiving )
unsigned long RCSwitch::nReceivedValue = 0;
unsigned int RCSwitch::nReceivedBitlength = 0;
unsigned int RCSwitch::nReceivedDelay = 0;
unsigned int RCSwitch::nReceivedProtocol = 0;
int RCSwitch::nReceiveTolerance = 60;
const unsigned int RCSwitch::nSeparationLimit = 4600;
// separationLimit: minimum microseconds between received codes, closer codes are ignored.
// according to discussion on issue #14 it might be more suitable to set the separation
// limit to the same time as the 'low' part of the sync signal for the current protocol.
unsigned int RCSwitch::timings[RCSWITCH_MAX_CHANGES];
#endif

RCSwitch::RCSwitch() {
  this->nTransmitterPin = -1;
  this->setRepeatTransmit(10);
  this->setProtocol(1);
  #if not defined( RCSwitchDisableReceiving )
  this->nReceiverInterrupt = -1;
  this->setReceiveTolerance(60);
  RCSwitch::nReceivedValue = 0;
  #endif
}

/**
  * Sets the protocol to send.
  */
void RCSwitch::setProtocol(Protocol protocol) {
  this->protocol = protocol;
}

/**
  * Sets the protocol to send, from a list of predefined protocols
  */
void RCSwitch::setProtocol(int nProtocol) {
  if (nProtocol < 1 || nProtocol > numProto) {
    nProtocol = 1;  // TODO: trigger an error, e.g. "bad protocol" ???
  }
  memcpy_P(&this->protocol, &proto[nProtocol-1], sizeof(Protocol));
}

/**
  * Sets the protocol to send with pulse length in microseconds.
  */
void RCSwitch::setProtocol(int nProtocol, int nPulseLength) {
  setProtocol(nProtocol);
  this->setPulseLength(nPulseLength);
}


/**
  * Sets pulse length in microseconds
  */
void RCSwitch::setPulseLength(int nPulseLength) {
  this->protocol.pulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void RCSwitch::setRepeatTransmit(int nRepeatTransmit) {
  this->nRepeatTransmit = nRepeatTransmit;
}

/**
 * Set Receiving Tolerance
 */
#if not defined( RCSwitchDisableReceiving )
void RCSwitch::setReceiveTolerance(int nPercent) {
  RCSwitch::nReceiveTolerance = nPercent;
}
#endif
  

/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RCSwitch::enableTransmit(int nTransmitterPin) {
  this->nTransmitterPin = nTransmitterPin;
  pinMode(this->nTransmitterPin, OUTPUT);
}

/**
  * Disable transmissions
  */
void RCSwitch::disableTransmit() {
  this->nTransmitterPin = -1;
}

/**
 * Switch a remote switch on (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitch::switchOn(char sGroup, int nDevice) {
  this->sendTriState( this->getCodeWordD(sGroup, nDevice, true) );
}

/**
 * Switch a remote switch off (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitch::switchOff(char sGroup, int nDevice) {
  this->sendTriState( this->getCodeWordD(sGroup, nDevice, false) );
}

/**
 * Switch a remote switch on (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
  */
void RCSwitch::switchOn(char sFamily, int nGroup, int nDevice) {
  this->sendTriState( this->getCodeWordC(sFamily, nGroup, nDevice, true) );
}

/**
 * Switch a remote switch off (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
 */
void RCSwitch::switchOff(char sFamily, int nGroup, int nDevice) {
  this->sendTriState( this->getCodeWordC(sFamily, nGroup, nDevice, false) );
}

/**
 * Switch a remote switch on (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOn(int nAddressCode, int nChannelCode) {
  this->sendTriState( this->getCodeWordB(nAddressCode, nChannelCode, true) );
}

/**
 * Switch a remote switch off (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOff(int nAddressCode, int nChannelCode) {
  this->sendTriState( this->getCodeWordB(nAddressCode, nChannelCode, false) );
}

/**
 * Deprecated, use switchOn(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitch::switchOn(const char* sGroup, int nChannel) {
  const char* code[6] = { "00000", "10000", "01000", "00100", "00010", "00001" };
  this->switchOn(sGroup, code[nChannel]);
}

/**
 * Deprecated, use switchOff(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitch::switchOff(const char* sGroup, int nChannel) {
  const char* code[6] = { "00000", "10000", "01000", "00100", "00010", "00001" };
  this->switchOff(sGroup, code[nChannel]);
}

/**
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitch::switchOn(const char* sGroup, const char* sDevice) {
    this->sendTriState( this->getCodeWordA(sGroup, sDevice, true) );
}

/**
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitch::switchOff(const char* sGroup, const char* sDevice) {
    this->sendTriState( this->getCodeWordA(sGroup, sDevice, false) );
}

/**
 * Returns a char[13], representing the code word to be sent.
 * A code word consists of 9 address bits, 3 data bits and one sync bit but
 * in our case only the first 8 address bits and the last 2 data bits were used.
 * A code bit can have 4 different states: "F" (floating), "0" (low), "1" (high), "S" (sync bit)
 *
 * +-----------------------------+-----------------------------+----------+----------+--------------+----------+
 * | 4 bits address              | 4 bits address              | 1 bit    | 1 bit    | 2 bits       | 1 bit    |
 * | switch group                | switch number               | not used | not used | on / off     | sync bit |
 * | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | F        | F        | on=FF off=F0 | S        |
 * +-----------------------------+-----------------------------+----------+----------+--------------+----------+
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13]
 */
char* RCSwitch::getCodeWordB(int nAddressCode, int nChannelCode, boolean bStatus) {
   int nReturnPos = 0;
   static char sReturn[13];
   
   const char* code[5] = { "FFFF", "0FFF", "F0FF", "FF0F", "FFF0" };
   if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4) {
    return '\0';
   }
   for (int i = 0; i<4; i++) {
     sReturn[nReturnPos++] = code[nAddressCode][i];
   }

   for (int i = 0; i<4; i++) {
     sReturn[nReturnPos++] = code[nChannelCode][i];
   }
   
   sReturn[nReturnPos++] = 'F';
   sReturn[nReturnPos++] = 'F';
   sReturn[nReturnPos++] = 'F';
   
   if (bStatus) {
      sReturn[nReturnPos++] = 'F';
   } else {
      sReturn[nReturnPos++] = '0';
   }
   
   sReturn[nReturnPos] = '\0';
   
   return sReturn;
}

/**
 * Returns a char[13], representing the code word to be send.
 *
 */
char* RCSwitch::getCodeWordA(const char* sGroup, const char* sDevice, boolean bOn) {
    static char sDipSwitches[13];
    int i = 0;
    int j = 0;
    
    for (i = 0; i < 5; i++) {
        sDipSwitches[j++] = (sGroup[i] == '0') ? 'F' : '0';
    }

    for (i = 0; i < 5; i++) {
        sDipSwitches[j++] = (sDevice[i] == '0') ? 'F' : '0';
    }

    if (bOn) {
        sDipSwitches[j++] = '0';
        sDipSwitches[j++] = 'F';
    } else {
        sDipSwitches[j++] = 'F';
        sDipSwitches[j++] = '0';
    }

    sDipSwitches[j] = '\0';

    return sDipSwitches;
}

/**
 * Like getCodeWord (Type C = Intertechno)
 */
char* RCSwitch::getCodeWordC(char sFamily, int nGroup, int nDevice, boolean bStatus) {
  static char sReturn[13];
  int nReturnPos = 0;
  
  if ( (byte)sFamily < 97 || (byte)sFamily > 112 || nGroup < 1 || nGroup > 4 || nDevice < 1 || nDevice > 4) {
    return '\0';
  }
  
  const char* sDeviceGroupCode =  dec2binWcharfill(  (nDevice-1) + (nGroup-1)*4, 4, '0'  );
  const char familycode[16][5] = {
      "0000", "F000", "0F00", "FF00",
      "00F0", "F0F0", "0FF0", "FFF0",
      "000F", "F00F", "0F0F", "FF0F",
      "00FF", "F0FF", "0FFF", "FFFF"
      };
  for (int i = 0; i<4; i++) {
    sReturn[nReturnPos++] = familycode[ (int)sFamily - 97 ][i];
  }
  for (int i = 0; i<4; i++) {
    sReturn[nReturnPos++] = (sDeviceGroupCode[3-i] == '1' ? 'F' : '0');
  }
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  if (bStatus) {
    sReturn[nReturnPos++] = 'F';
  } else {
    sReturn[nReturnPos++] = '0';
  }
  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Decoding for the REV Switch Type
 *
 * Returns a char[13], representing the Tristate to be send.
 * A Code Word consists of 7 address bits and 5 command data bits.
 * A Code Bit can have 3 different states: "F" (floating), "0" (low), "1" (high)
 *
 * +-------------------------------+--------------------------------+-----------------------+
 * | 4 bits address (switch group) | 3 bits address (device number) | 5 bits (command data) |
 * | A=1FFF B=F1FF C=FF1F D=FFF1   | 1=0FFF 2=F0FF 3=FF0F 4=FFF0    | on=00010 off=00001    |
 * +-------------------------------+--------------------------------+-----------------------+
 *
 * Source: http://www.the-intruder.net/funksteckdosen-von-rev-uber-arduino-ansteuern/
 *
 * @param sGroup        Name of the switch group (A..D, resp. a..d) 
 * @param nDevice       Number of the switch itself (1..3)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13]
 */

char* RCSwitch::getCodeWordD(char sGroup, int nDevice, boolean bStatus){
    static char sReturn[13];
    int nReturnPos = 0;

    // Building 4 bits address
    // (Potential problem if dec2binWcharfill not returning correct string)
    char *sGroupCode;
    switch(sGroup){
        case 'a':
        case 'A':
            sGroupCode = dec2binWcharfill(8, 4, 'F'); break;
        case 'b':
        case 'B':
            sGroupCode = dec2binWcharfill(4, 4, 'F'); break;
        case 'c':
        case 'C':
            sGroupCode = dec2binWcharfill(2, 4, 'F'); break;
        case 'd':
        case 'D':
            sGroupCode = dec2binWcharfill(1, 4, 'F'); break;
        default:
            return '\0';
    }
    
    for (int i = 0; i<4; i++) {
        sReturn[nReturnPos++] = sGroupCode[i];
    }


    // Building 3 bits address
    // (Potential problem if dec2binWcharfill not returning correct string)
    char *sDevice;
    switch(nDevice) {
        case 1:
            sDevice = dec2binWcharfill(4, 3, 'F'); break;
        case 2:
            sDevice = dec2binWcharfill(2, 3, 'F'); break;
        case 3:
            sDevice = dec2binWcharfill(1, 3, 'F'); break;
        default:
            return '\0';
    }

    for (int i = 0; i<3; i++)
        sReturn[nReturnPos++] = sDevice[i];

    // fill up rest with zeros
    for (int i = 0; i<5; i++)
        sReturn[nReturnPos++] = '0';

    // encode on or off
    if (bStatus)
        sReturn[10] = '1';
    else
        sReturn[11] = '1';

    // last position terminate string
    sReturn[12] = '\0';
    return sReturn;

}

/**
 * @param sCodeWord   /^[10FS]*$/  -> see getCodeWord
 */
void RCSwitch::sendTriState(const char* sCodeWord) {
  for (int nRepeat=0; nRepeat<nRepeatTransmit; nRepeat++) {
    int i = 0;
    while (sCodeWord[i] != '\0') {
      switch(sCodeWord[i]) {
        case '0':
          this->sendT0();
        break;
        case 'F':
          this->sendTF();
        break;
        case '1':
          this->sendT1();
        break;
      }
      i++;
    }
    this->sendSync();    
  }
}

void RCSwitch::send(unsigned long code, unsigned int length) {
  this->send( this->dec2binWcharfill(code, length, '0') );
}

void RCSwitch::send(const char* sCodeWord) {
  for (int nRepeat=0; nRepeat<nRepeatTransmit; nRepeat++) {
    int i = 0;
    while (sCodeWord[i] != '\0') {
      switch(sCodeWord[i]) {
        case '0':
          this->send0();
        break;
        case '1':
          this->send1();
        break;
      }
      i++;
    }
    this->sendSync();
  }
}

void RCSwitch::transmit(int nHighPulses, int nLowPulses) {
    if (this->nTransmitterPin != -1) {
        #if not defined( RCSwitchDisableReceiving )
        int nReceiverInterrupt_backup = nReceiverInterrupt;
        if (nReceiverInterrupt_backup != -1) {
            this->disableReceive();
        }
        #endif
        digitalWrite(this->nTransmitterPin, HIGH);
        delayMicroseconds( this->protocol.pulseLength * nHighPulses);
        digitalWrite(this->nTransmitterPin, LOW);
        delayMicroseconds( this->protocol.pulseLength * nLowPulses);
        
        #if not defined( RCSwitchDisableReceiving )
        if (nReceiverInterrupt_backup != -1) {
            this->enableReceive(nReceiverInterrupt_backup);
        }
        #endif
    }
}

void RCSwitch::transmit(HighLow pulses) {
    transmit(pulses.high, pulses.low);
}

/**
 * Sends a "0" Bit
 *                       _    
 * Waveform Protocol 1: | |___
 *                       _  
 * Waveform Protocol 2: | |__
 */
void RCSwitch::send0() {
    this->transmit(protocol.zero);
}

/**
 * Sends a "1" Bit
 *                       ___  
 * Waveform Protocol 1: |   |_
 *                       __  
 * Waveform Protocol 2: |  |_
 */
void RCSwitch::send1() {
    this->transmit(protocol.one);
}


/**
 * Sends a Tri-State "0" Bit
 *            _     _
 * Waveform: | |___| |___
 */
void RCSwitch::sendT0() {
  this->send0();
  this->send0();
}

/**
 * Sends a Tri-State "1" Bit
 *            ___   ___
 * Waveform: |   |_|   |_
 */
void RCSwitch::sendT1() {
  this->send1();
  this->send1();
}

/**
 * Sends a Tri-State "F" Bit
 *            _     ___
 * Waveform: | |___|   |_
 */
void RCSwitch::sendTF() {
  this->send0();
  this->send1();
}

/**
 * Sends a "Sync" Bit
 *                       _
 * Waveform Protocol 1: | |_______________________________
 *                       _
 * Waveform Protocol 2: | |__________
 */
void RCSwitch::sendSync() {
    this->transmit(protocol.syncFactor);
}

#if not defined( RCSwitchDisableReceiving )
/**
 * Enable receiving data
 */
void RCSwitch::enableReceive(int interrupt) {
  this->nReceiverInterrupt = interrupt;
  this->enableReceive();
}

void RCSwitch::enableReceive() {
  if (this->nReceiverInterrupt != -1) {
    RCSwitch::nReceivedValue = 0;
    RCSwitch::nReceivedBitlength = 0;
    attachInterrupt(this->nReceiverInterrupt, handleInterrupt, CHANGE);
  }
}

/**
 * Disable receiving data
 */
void RCSwitch::disableReceive() {
  detachInterrupt(this->nReceiverInterrupt);
  this->nReceiverInterrupt = -1;
}

bool RCSwitch::available() {
  return RCSwitch::nReceivedValue != 0;
}

void RCSwitch::resetAvailable() {
  RCSwitch::nReceivedValue = 0;
}

unsigned long RCSwitch::getReceivedValue() {
    return RCSwitch::nReceivedValue;
}

unsigned int RCSwitch::getReceivedBitlength() {
  return RCSwitch::nReceivedBitlength;
}

unsigned int RCSwitch::getReceivedDelay() {
  return RCSwitch::nReceivedDelay;
}

unsigned int RCSwitch::getReceivedProtocol() {
  return RCSwitch::nReceivedProtocol;
}

unsigned int* RCSwitch::getReceivedRawdata() {
    return RCSwitch::timings;
}

/* helper function for the various receiveProtocol methods */
static inline unsigned int diff(int A, int B) {
    return abs(A - B);
}

/**
 *
 */
bool RCSwitch::receiveProtocol(const int p, unsigned int changeCount) {

    Protocol pro;
    memcpy_P(&pro, &proto[p-1], sizeof(Protocol));

    unsigned long code = 0;
    const unsigned int delay = RCSwitch::timings[0] / pro.syncFactor.low;
    const unsigned int delayTolerance = delay * RCSwitch::nReceiveTolerance / 100;

    for (unsigned int i = 1; i < changeCount; i += 2) {
        code <<= 1;
        if (diff(RCSwitch::timings[i], delay * pro.zero.high) < delayTolerance &&
            diff(RCSwitch::timings[i + 1], delay * pro.zero.low) < delayTolerance) {
            // zero
        } else if (diff(RCSwitch::timings[i], delay * pro.one.high) < delayTolerance &&
                   diff(RCSwitch::timings[i + 1], delay * pro.one.low) < delayTolerance) {
            // one
            code |= 1;
        } else {
            // Failed
            return false;
        }
    }

    if (changeCount > 6) {    // ignore < 4bit values as there are no devices sending 4bit values => noise
        RCSwitch::nReceivedValue = code;
        RCSwitch::nReceivedBitlength = changeCount / 2;
        RCSwitch::nReceivedDelay = delay;
        RCSwitch::nReceivedProtocol = p;
    }

    return true;
}

void RCSwitch::handleInterrupt() {

  static unsigned int duration;
  static unsigned int changeCount;
  static unsigned long lastTime;
  static unsigned int repeatCount;
  

  long time = micros();
  duration = time - lastTime;
 
  if (duration > RCSwitch::nSeparationLimit && diff(duration, RCSwitch::timings[0]) < 200) {
    repeatCount++;
    changeCount--;
    if (repeatCount == 2) {
      if (receiveProtocol(1, changeCount) == false) {
        if (receiveProtocol(2, changeCount) == false) {
          if (receiveProtocol(3, changeCount) == false) {
            //failed
          }
        }
      }
      repeatCount = 0;
    }
    changeCount = 0;
  } else if (duration > RCSwitch::nSeparationLimit) {
    changeCount = 0;
  }
 
  if (changeCount >= RCSWITCH_MAX_CHANGES) {
    changeCount = 0;
    repeatCount = 0;
  }
  RCSwitch::timings[changeCount++] = duration;
  lastTime = time;  
}
#endif

/**
  * Turns a decimal value to its binary representation
  */
char* RCSwitch::dec2binWcharfill(unsigned long dec, unsigned int bitLength, char fill) {
  static char bin[32];

  bin[bitLength] = '\0';
  while (bitLength > 0) {
    bitLength--;
    bin[bitLength] = (dec & 1) ? '1' : fill;
    dec >>= 1;
  }

  return bin;
}
