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
  - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com
  - Johann Richard / <first name>.<last name>(at)gmail(dot)com
  - Vlad Gheorghe / <first name>.<last name>(at)gmail(dot)com https://github.com/vgheo
  - Per Ivar Nerseth / <first name>(at)<last name>(dot)com
  
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

//#define DEBUG //If you comment this line, debug statements are turned off

#ifdef RaspberryPi
// PROGMEM and _P functions are for AVR based microprocessors,
// so we must normalize these for the ARM processor:
#define PROGMEM
#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#endif

#if defined(ESP8266) || defined(ESP32)
// interrupt handler and related code must be in RAM on ESP8266,
// according to issue #46.
#define RECEIVE_ATTR ICACHE_RAM_ATTR
#else
#define RECEIVE_ATTR
#endif

/* Format for protocol definitions:
 * { 
 *  pulselength, 
 *  sync bit before the data bits. Normally zero, i.e. {0, 0},
 *  "0" bit, 
 *  "1" bit, 
 *  pause bit,
 *  bool inverted signal
 * }
 * 
 * pulselength: pulse length in microseconds, e.g. 350
 * pause/sync bit: {1, 31} means 1 high pulse and 31 low pulses
 *     (perceived as a 31*pulselength long pulse, total length of sync bit is
 *     32*pulselength microseconds), i.e:
 *      _
 *     | |_______________________________ (don't count the vertical bars)
 * "0" bit: waveform for a data bit of value "0", {1, 3} means 1 high pulse
 *     and 3 low pulses, total length (1+3)*pulselength, i.e:
 *      _
 *     | |___
 * "1" bit: waveform for a data bit of value "1", e.g. {3,1}:
 *      ___
 *     |   |_
 *
 * These are combined to form Tri-State bits when sending or receiving codes.
 */
#if defined(ESP8266) || defined(ESP32)
static const RCSwitch::Protocol proto[] = {
#else
static const RCSwitch::Protocol PROGMEM proto[] = {
#endif
    {350, {0, 0}, {1, 3}, {3, 1}, {1, 31}, false},   // protocol 1
    {650, {0, 0}, {1, 2}, {2, 1}, {1, 10}, false},   // protocol 2
    {100, {0, 0}, {4, 11}, {9, 6}, {30, 71}, false}, // protocol 3
    {380, {0, 0}, {1, 3}, {3, 1}, {1, 6}, false},    // protocol 4
    {500, {0, 0}, {1, 2}, {2, 1}, {6, 14}, false},   // protocol 5
    {450, {0, 0}, {1, 2}, {2, 1}, {23, 1}, true},    // protocol 6 (HT6P20B)
    {150, {0, 0}, {1, 6}, {6, 1}, {2, 62}, false},   // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
    {250, {1, 10}, {1, 5}, {1, 1}, {1, 40}, false},  // protocol 8 (Nexa)
    {100, {0, 0}, {6, 6}, {6, 12}, {6, 169}, false}, // protocol 9 (Everflourish Single Button)
    {100, {0, 0}, {6, 6}, {6, 12}, {6, 120}, false}, // protocol 10 (Everflourish All Buttons)
};

enum
{
  numProto = sizeof(proto) / sizeof(proto[0])
};

#if not defined(RCSwitchDisableReceiving)
volatile unsigned long RCSwitch::nReceivedValue = 0;
volatile unsigned int RCSwitch::nReceivedBitlength = 0;
volatile unsigned int RCSwitch::nReceivedDelay = 0;
volatile unsigned int RCSwitch::nReceivedProtocol = 0;
int RCSwitch::nReceiveTolerance = 60;
const unsigned int RCSwitch::nSeparationLimit = 4300;
// separationLimit: minimum microseconds between received codes, closer codes are ignored.
// according to discussion on issue #14 it might be more suitable to set the separation
// limit to the same time as the 'low' part of the sync signal for the current protocol.
unsigned int RCSwitch::timings[RCSWITCH_MAX_CHANGES];
unsigned int RCSwitch::timings_copy[RCSWITCH_MAX_CHANGES];

// array to hold the bits received as chars (to support longer frames)
char RCSwitch::receivedBits[RCSWITCH_MAX_CHANGES];
#endif

RCSwitch::RCSwitch()
{
  this->nTransmitterPin = -1;
  this->setRepeatTransmit(10);
  this->setProtocol(1);
#if not defined(RCSwitchDisableReceiving)
  this->nReceiverInterrupt = -1;
  this->setReceiveTolerance(60);
  RCSwitch::nReceivedValue = 0;
#endif
}

/**
  * Sets the protocol to send.
  */
void RCSwitch::setProtocol(Protocol protocol)
{
  this->protocol = protocol;
}

/**
  * Sets the protocol to send, from a list of predefined protocols
  */
void RCSwitch::setProtocol(int nProtocol)
{
  if (nProtocol < 1 || nProtocol > numProto)
  {
    nProtocol = 1; // TODO: trigger an error, e.g. "bad protocol" ???
  }
#if defined(ESP8266) || defined(ESP32)
  this->protocol = proto[nProtocol - 1];
#else
  memcpy_P(&this->protocol, &proto[nProtocol - 1], sizeof(Protocol));
#endif
}

/**
  * Sets the protocol to send with pulse length in microseconds.
  */
void RCSwitch::setProtocol(int nProtocol, int nPulseLength)
{
  setProtocol(nProtocol);
  this->setPulseLength(nPulseLength);
}

/**
  * gets the protocol
  */
RCSwitch::Protocol RCSwitch::getProtocol()
{
  return this->protocol;
}

/**
  * Sets pulse length in microseconds
  */
void RCSwitch::setPulseLength(int nPulseLength)
{
  this->protocol.pulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void RCSwitch::setRepeatTransmit(int nRepeatTransmit)
{
  this->nRepeatTransmit = nRepeatTransmit;
}

/**
 * Set Receiving Tolerance
 */
#if not defined(RCSwitchDisableReceiving)
void RCSwitch::setReceiveTolerance(int nPercent)
{
  RCSwitch::nReceiveTolerance = nPercent;
}
#endif

/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RCSwitch::enableTransmit(int nTransmitterPin)
{
  this->nTransmitterPin = nTransmitterPin;
  pinMode(this->nTransmitterPin, OUTPUT);
}

/**
  * Disable transmissions
  */
void RCSwitch::disableTransmit()
{
  this->nTransmitterPin = -1;
}

/**
 * Switch a remote switch on (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitch::switchOn(char sGroup, int nDevice)
{
  this->sendTriState(this->getCodeWordD(sGroup, nDevice, true));
}

/**
 * Switch a remote switch off (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitch::switchOff(char sGroup, int nDevice)
{
  this->sendTriState(this->getCodeWordD(sGroup, nDevice, false));
}

/**
 * Switch a remote switch on (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
  */
void RCSwitch::switchOn(char sFamily, int nGroup, int nDevice)
{
  this->sendTriState(this->getCodeWordC(sFamily, nGroup, nDevice, true));
}

/**
 * Switch a remote switch off (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
 */
void RCSwitch::switchOff(char sFamily, int nGroup, int nDevice)
{
  this->sendTriState(this->getCodeWordC(sFamily, nGroup, nDevice, false));
}

/**
 * Switch a remote switch on (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOn(int nAddressCode, int nChannelCode)
{
  this->sendTriState(this->getCodeWordB(nAddressCode, nChannelCode, true));
}

/**
 * Switch a remote switch off (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitch::switchOff(int nAddressCode, int nChannelCode)
{
  this->sendTriState(this->getCodeWordB(nAddressCode, nChannelCode, false));
}

/**
 * Deprecated, use switchOn(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitch::switchOn(const char *sGroup, int nChannel)
{
  const char *code[6] = {"00000", "10000", "01000", "00100", "00010", "00001"};
  this->switchOn(sGroup, code[nChannel]);
}

/**
 * Deprecated, use switchOff(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitch::switchOff(const char *sGroup, int nChannel)
{
  const char *code[6] = {"00000", "10000", "01000", "00100", "00010", "00001"};
  this->switchOff(sGroup, code[nChannel]);
}

/**
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitch::switchOn(const char *sGroup, const char *sDevice)
{
  this->sendTriState(this->getCodeWordA(sGroup, sDevice, true));
}

/**
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitch::switchOff(const char *sGroup, const char *sDevice)
{
  this->sendTriState(this->getCodeWordA(sGroup, sDevice, false));
}

/**
 * Returns a char[13], representing the code word to be send.
 *
 */
char *RCSwitch::getCodeWordA(const char *sGroup, const char *sDevice, bool bStatus)
{
  static char sReturn[13];
  int nReturnPos = 0;

  for (int i = 0; i < 5; i++)
  {
    sReturn[nReturnPos++] = (sGroup[i] == '0') ? 'F' : '0';
  }

  for (int i = 0; i < 5; i++)
  {
    sReturn[nReturnPos++] = (sDevice[i] == '0') ? 'F' : '0';
  }

  sReturn[nReturnPos++] = bStatus ? '0' : 'F';
  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Encoding for type B switches with two rotary/sliding switches.
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-----------------------------+----------+------------+
 * | 4 bits address              | 4 bits address              | 3 bits   | 1 bit      |
 * | switch group                | switch number               | not used | on / off   |
 * | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | FFF      | on=F off=0 |
 * +-----------------------------+-----------------------------+----------+------------+
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char *RCSwitch::getCodeWordB(int nAddressCode, int nChannelCode, bool bStatus)
{
  static char sReturn[13];
  int nReturnPos = 0;

  if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4)
  {
    return 0;
  }

  for (int i = 1; i <= 4; i++)
  {
    sReturn[nReturnPos++] = (nAddressCode == i) ? '0' : 'F';
  }

  for (int i = 1; i <= 4; i++)
  {
    sReturn[nReturnPos++] = (nChannelCode == i) ? '0' : 'F';
  }

  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';

  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Like getCodeWord (Type C = Intertechno)
 */
char *RCSwitch::getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus)
{
  static char sReturn[13];
  int nReturnPos = 0;

  int nFamily = (int)sFamily - 'a';
  if (nFamily < 0 || nFamily > 15 || nGroup < 1 || nGroup > 4 || nDevice < 1 || nDevice > 4)
  {
    return 0;
  }

  // encode the family into four bits
  sReturn[nReturnPos++] = (nFamily & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 2) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 4) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 8) ? 'F' : '0';

  // encode the device and group
  sReturn[nReturnPos++] = ((nDevice - 1) & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nDevice - 1) & 2) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nGroup - 1) & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nGroup - 1) & 2) ? 'F' : '0';

  // encode the status code
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Encoding for the REV Switch Type
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-------------------+----------+--------------+
 * | 4 bits address              | 3 bits address    | 3 bits   | 2 bits       |
 * | switch group                | device number     | not used | on / off     |
 * | A=1FFF B=F1FF C=FF1F D=FFF1 | 1=0FF 2=F0F 3=FF0 | 000      | on=10 off=01 |
 * +-----------------------------+-------------------+----------+--------------+
 *
 * Source: http://www.the-intruder.net/funksteckdosen-von-rev-uber-arduino-ansteuern/
 *
 * @param sGroup        Name of the switch group (A..D, resp. a..d) 
 * @param nDevice       Number of the switch itself (1..3)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char *RCSwitch::getCodeWordD(char sGroup, int nDevice, bool bStatus)
{
  static char sReturn[13];
  int nReturnPos = 0;

  // sGroup must be one of the letters in "abcdABCD"
  int nGroup = (sGroup >= 'a') ? (int)sGroup - 'a' : (int)sGroup - 'A';
  if (nGroup < 0 || nGroup > 3 || nDevice < 1 || nDevice > 3)
  {
    return 0;
  }

  for (int i = 0; i < 4; i++)
  {
    sReturn[nReturnPos++] = (nGroup == i) ? '1' : 'F';
  }

  for (int i = 1; i <= 3; i++)
  {
    sReturn[nReturnPos++] = (nDevice == i) ? '1' : 'F';
  }

  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';

  sReturn[nReturnPos++] = bStatus ? '1' : '0';
  sReturn[nReturnPos++] = bStatus ? '0' : '1';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * @param sCodeWord   a tristate code word consisting of the letter 0, 1, F
 */
void RCSwitch::sendTriState(const char *sCodeWord)
{
  // turn the tristate code word into the corresponding bit pattern, then send it
  unsigned long code = 0;
  unsigned int length = 0;
  for (const char *p = sCodeWord; *p; p++)
  {
    code <<= 2L;
    switch (*p)
    {
    case '0':
      // bit pattern 00
      break;
    case 'F':
      // bit pattern 01
      code |= 1L;
      break;
    case '1':
      // bit pattern 11
      code |= 3L;
      break;
    }
    length += 2;
  }
  this->send(code, length);
}

/**
 * @param sBitString   a binary string consisting of the letters 0, 1
 */
void RCSwitch::send(const char *sBitString)
{
  if (this->nTransmitterPin == -1)
    return;

#if not defined(RCSwitchDisableReceiving)
  // make sure the receiver is disabled while we transmit
  int nReceiverInterrupt_backup = nReceiverInterrupt;
  if (nReceiverInterrupt_backup != -1)
  {
    this->disableReceive();
  }
#endif

  for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++)
  {
    // transmit sync bits at the beginning if they are defined
    if (protocol.sync.high > 0 && protocol.sync.low > 0)
    {
      this->transmit(protocol.sync);
    }

    // transmit the data bits
    for (const char *p = sBitString; *p; p++)
    {
      if (*p != '0')
      {
        this->transmit(protocol.one);
      }
      else
      {
        this->transmit(protocol.zero);
      }
    }

    // transmit the pause bits at the end
    this->transmit(protocol.pause);
  }

  // Disable transmit after sending (i.e., for inverted protocols)
  digitalWrite(this->nTransmitterPin, LOW);

#if not defined(RCSwitchDisableReceiving)
  // enable receiver again if we just disabled it
  if (nReceiverInterrupt_backup != -1)
  {
    this->enableReceive(nReceiverInterrupt_backup);
  }
#endif
}

/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void RCSwitch::send(unsigned long code, unsigned int length)
{
  if (this->nTransmitterPin == -1)
    return;

#if not defined(RCSwitchDisableReceiving)
  // make sure the receiver is disabled while we transmit
  int nReceiverInterrupt_backup = nReceiverInterrupt;
  if (nReceiverInterrupt_backup != -1)
  {
    this->disableReceive();
  }
#endif

  for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++)
  {
    for (int i = length - 1; i >= 0; i--)
    {
      if (code & (1L << i))
        this->transmit(protocol.one);
      else
        this->transmit(protocol.zero);
    }
    this->transmit(protocol.pause);
  }

  // Disable transmit after sending (i.e., for inverted protocols)
  digitalWrite(this->nTransmitterPin, LOW);

#if not defined(RCSwitchDisableReceiving)
  // enable receiver again if we just disabled it
  if (nReceiverInterrupt_backup != -1)
  {
    this->enableReceive(nReceiverInterrupt_backup);
  }
#endif
}

/**
 * Transmit a single high-low pulse.
 */
void RCSwitch::transmit(HighLow pulses)
{
  uint8_t firstLogicLevel = (this->protocol.invertedSignal) ? LOW : HIGH;
  uint8_t secondLogicLevel = (this->protocol.invertedSignal) ? HIGH : LOW;

  // delayMicroseconds doesn't support values above 16383
  // so use a while loop instead of delayMicroseconds
  unsigned long microsDelayHigh = this->protocol.pulseLength * pulses.high;
  unsigned long microsDelayLow = this->protocol.pulseLength * pulses.low;
  unsigned long startMicros = 0;

  // transmit "high" or the inverted signal
  digitalWrite(this->nTransmitterPin, firstLogicLevel);
  startMicros = micros();
  while (micros() - startMicros < microsDelayHigh)
    continue;

  // transmit "low" or the inverted signal
  digitalWrite(this->nTransmitterPin, secondLogicLevel);
  startMicros = micros();
  while (micros() - startMicros < microsDelayLow)
    continue;
}

#if not defined(RCSwitchDisableReceiving)
/**
 * Enable receiving data
 */
void RCSwitch::enableReceive(int interrupt)
{
  this->nReceiverInterrupt = interrupt;
  this->enableReceive();
}

void RCSwitch::enableReceive()
{
  if (this->nReceiverInterrupt != -1)
  {
    RCSwitch::nReceivedValue = 0;
    RCSwitch::nReceivedBitlength = 0;
#if defined(RaspberryPi) // Raspberry Pi
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
#else // Arduino
    attachInterrupt(this->nReceiverInterrupt, handleInterrupt, CHANGE);
#endif
  }
}

int RCSwitch::getReceiverInterrupt()
{
  return RCSwitch::nReceiverInterrupt;
}

/**
 * Disable receiving data
 */
void RCSwitch::disableReceive()
{
#if not defined(RaspberryPi) // Arduino
  detachInterrupt(this->nReceiverInterrupt);
#endif // For Raspberry Pi (wiringPi) you can't unregister the ISR
  this->nReceiverInterrupt = -1;
}

bool RCSwitch::available()
{
  return RCSwitch::nReceivedValue != 0;
}

void RCSwitch::resetAvailable()
{
  RCSwitch::nReceivedValue = 0;
}

unsigned long RCSwitch::getReceivedValue()
{
  return RCSwitch::nReceivedValue;
}

unsigned int RCSwitch::getReceivedBitlength()
{
  return RCSwitch::nReceivedBitlength;
}

unsigned int RCSwitch::getReceivedDelay()
{
  return RCSwitch::nReceivedDelay;
}

unsigned int RCSwitch::getReceivedProtocol()
{
  return RCSwitch::nReceivedProtocol;
}

/* use getReceivedBitlength() * 2 to output the number of raw bits */
unsigned int *RCSwitch::getReceivedRawdata()
{
  return RCSwitch::timings_copy;
}

/* use getReceivedBitlength() to output the number of raw bits */
char *RCSwitch::getReceivedRawBits()
{
  return RCSwitch::receivedBits;
}

/* helper function for the receiveProtocol method */
static inline unsigned int diff(int A, int B)
{
  return abs(A - B);
}

/* helper function for debugging decoding binary inputs (used in the receiveProtocol method) */
char *RCSwitch::dec2binWzerofill(unsigned long Dec, unsigned int bitLength)
{
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0)
  {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++)
  {
    if (j >= bitLength - i)
    {
      bin[j] = bin[31 + i - (j - (bitLength - i))];
    }
    else
    {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}

/**
 *
 */
bool RECEIVE_ATTR RCSwitch::receiveProtocol(const int p, unsigned int changeCount)
{
  if (changeCount <= 7)
  { // ignore very short transmissions: no device sends them, so this must be noise
    return false;
  }

#if defined(ESP8266) || defined(ESP32)
  const Protocol &pro = proto[p - 1];
#else
  Protocol pro;
  memcpy_P(&pro, &proto[p - 1], sizeof(Protocol));
#endif

  unsigned long code = 0;
  //Assuming the longer pulse length is the pulse captured in timings[0]
  const unsigned int pauseLengthInPulses = ((pro.pause.low) > (pro.pause.high)) ? (pro.pause.low) : (pro.pause.high);
  const unsigned int delay = RCSwitch::timings[0] / pauseLengthInPulses;
  const unsigned int delayTolerance = delay * RCSwitch::nReceiveTolerance / 100;

  // store bits in the receivedBits char array (to support longer frames)
  // if we have sync bits (like Nexa), don't count the initial pause bit and the two sync bits
  // otherwise, just ignore the initial pause bit
  const unsigned int receivedBitlength = (pro.sync.high > 0 && pro.sync.low > 0) ? (changeCount - 3) / 2 : (changeCount - 1) / 2;
  unsigned int receivedBitsPos = 0;

  /* For protocols that start low, the pause period looks like
     *               _________
     * _____________|         |XXXXXXXXXXXX|
     *
     * |--1st dur--|-2nd dur-|-Start data-|
     *
     * The 3rd saved duration starts the data.
     *
     * For protocols that start high, the pause period looks like
     *
     *  ______________
     * |              |____________|XXXXXXXXXXXXX|
     *
     * |-filtered out-|--1st dur--|--Start data--|
     *
     * The 2nd saved duration starts the data
     */
  const unsigned int firstDataTiming = (pro.invertedSignal) ? (2) : (1);

#ifdef DEBUG
  if (p > 7) // debugging protocols 8 and 9
  {
    Serial.println();
    Serial.print(F("Testing if this is protocol "));
    Serial.print(p);
    Serial.print(F(" using "));
    Serial.print(changeCount);
    Serial.print(F(" timings. PauseLengthInPulses: "));
    Serial.print(pauseLengthInPulses);
    Serial.print(F(". Delay: "));
    Serial.print(delay);
    Serial.print(F(". Delay tolerance: "));
    Serial.print(delayTolerance);
    Serial.print(F(". Received Bitlength: "));
    Serial.print(receivedBitlength);
    Serial.print(F(". First Data Timing Index: "));
    Serial.print(firstDataTiming);
    Serial.println();

    Serial.print(F("Raw timing data: "));
    for (unsigned int p = 0; p < changeCount; p++)
    {
      Serial.print(RCSwitch::timings[p]);
      Serial.print(F(","));
    }
    Serial.println();
  }
#endif

  for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2)
  {
    // if the protocol contains a sync bit, check for it (e.g. Nexa)
    if ((pro.sync.high > 0 && pro.sync.low > 0) &&
        diff(RCSwitch::timings[i], delay * pro.sync.high) < delayTolerance &&
        diff(RCSwitch::timings[i + 1], delay * pro.sync.low) < delayTolerance)
    {
    // sync bit
#ifdef DEBUG
      if (p > 7) // debugging protocols 8 and 9
      {
        Serial.print(F("sync "));
      }
#endif
    }
    else if (diff(RCSwitch::timings[i], delay * pro.zero.high) < delayTolerance &&
             diff(RCSwitch::timings[i + 1], delay * pro.zero.low) < delayTolerance)
    {
    // zero
#ifdef DEBUG
      if (p > 7) // debugging protocols 8 and 9
      {
        Serial.print(F("0"));
      }
#endif
      // store zero in receivedBits char array
      RCSwitch::receivedBits[receivedBitsPos++] = '0';
    }
    else if (diff(RCSwitch::timings[i], delay * pro.one.high) < delayTolerance &&
             diff(RCSwitch::timings[i + 1], delay * pro.one.low) < delayTolerance)
    {
    // one
#ifdef DEBUG
      if (p > 7) // debugging protocols 8 and 9
      {
        Serial.print(F("1"));
      }
#endif
      // store one in receivedBits char array
      RCSwitch::receivedBits[receivedBitsPos++] = '1';
    }
    else
    {
#ifdef DEBUG
      if (p > 7) // debugging protocols 8 and 9
      {
        Serial.print(F(" failed(i="));
        Serial.print(i);
        Serial.print(F(")"));
        Serial.println();
      }
#endif
      // Failed
      return false;
    }
  }

#ifdef DEBUG
  // print check bit stream
  if (receivedBitsPos > 0)
  {
    Serial.println();
    Serial.print(F("vrfy "));
    for (unsigned int j = 0; j < receivedBitsPos; j++)
    {
      Serial.print(RCSwitch::receivedBits[j]);
    }
    Serial.println();
  }
#endif

  if (changeCount > 67)
  {
    // If large data stream, like Nexa.
    // Decode the data stream into logical bytes.
    // The data part on the physical link is coded so that every logical bit is sent as
    // two physical bits, where the second one is the inverse of the first one.
    // '0' => '01'
    // '1' => '10'
    // Example the logical datastream 0111 is sent over the air as 01101010.
    // I.e. the solution is to keep every second byte
    if (receivedBitsPos > 0)
    {
      for (unsigned int k = 0; k < receivedBitsPos; k += 2)
      {
        code <<= 1;
        if (RCSwitch::receivedBits[k] == '1')
        {
          code |= 1;
        }
      }
    }
  }
  else
  {
    // Decode the data stream into logical bytes.
    if (receivedBitsPos > 0)
    {
      for (unsigned int l = 0; l < receivedBitsPos; l++)
      {
        code <<= 1;
        if (RCSwitch::receivedBits[l] == '1')
        {
          code |= 1;
        }
      }
    }
  }

#ifdef DEBUG
  // print the decoded bit stream
  if (code > 0)
  {
    Serial.print(F("lgic "));
    Serial.println(RCSwitch::dec2binWzerofill(code, receivedBitlength / 2));
  }
#endif

  if (changeCount > 7)
  { // ignore very short transmissions: no device sends them, so this must be noise

    // copy the timings array to the timings_copy array, so that the
    // raw timings returned in the method getReceivedRawdata is not modified by
    // the interrupt-handler
    memcpy(timings_copy, timings, sizeof(timings));

    RCSwitch::nReceivedValue = code;
    RCSwitch::nReceivedBitlength = receivedBitlength;
    RCSwitch::nReceivedDelay = delay;
    RCSwitch::nReceivedProtocol = p;

#ifdef DEBUG
    Serial.println(F("Successfully found protocol, returning."));
#endif

    return true;
  }

  return false;
}

void RECEIVE_ATTR RCSwitch::handleInterrupt()
{
  static unsigned int changeCount = 0;
  static unsigned long lastTime = 0;
  static unsigned int repeatCount = 0;

  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (duration > RCSwitch::nSeparationLimit)
  {
    // A long stretch without signal level change occurred. This could
    // be the gap between two transmission.
    if (diff(duration, RCSwitch::timings[0]) < 200)
    {
      // This long signal is close in length to the long signal which
      // started the previously recorded timings; this suggests that
      // it may indeed by a a gap between two transmissions (we assume
      // here that a sender will send the signal multiple times,
      // with roughly the same gap between them).
      repeatCount++;
      if (repeatCount == 2)
      {
        for (unsigned int i = 1; i <= numProto; i++)
        {
          if (receiveProtocol(i, changeCount))
          {
            // receive succeeded for protocol i
            break;
          }
        }
        repeatCount = 0;
      }
    }
    changeCount = 0;
  }

  // detect overflow
  if (changeCount >= RCSWITCH_MAX_CHANGES)
  {
    changeCount = 0;
    repeatCount = 0;
  }

  // This stores the timings. Notes that it also overwrites the timings even if
  // receiveProtocol was successfull, so the returned raw timings (getReceivedRawdata)
  // will always have been modified after it has been used.
  // Therefore the timings are copied to a copy (timings_copy) before returned to the user.
  RCSwitch::timings[changeCount++] = duration;
  lastTime = time;
}
#endif
