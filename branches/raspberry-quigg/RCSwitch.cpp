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
  - Malte Diers / m<lastname>(at)gmail(dot)com

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

// Get rid of nasty warnings...
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "RCSwitch.h"

#ifndef RCSwitchDisableReceiving
unsigned long       RCSwitch::s_nReceivedValue     = 0;
unsigned int        RCSwitch::s_nReceivedBitlength = 0;
unsigned int        RCSwitch::s_nReceivedDelay     = 0;
RCSwitch::eProtocol RCSwitch::s_nReceivedProtocol  = RCSwitch::TYPE_UNDEF;
         int        RCSwitch::s_nReceiveTolerance  = 60;
unsigned int        RCSwitch::timings[RCSWITCH_MAX_CHANGES];
#endif // RCSwitchDisableReceiving

const char RCSwitch::TYPE_A_CODE[ 6][6]  = { "00000", "10000", "01000",
                                             "00100", "00010", "00001"
                                           };
const char RCSwitch::TYPE_B_CODE[ 5][5]  = { "FFFF", "0FFF", "F0FF", "FF0F", "FFF0" };
const char RCSwitch::TYPE_C_CODE[16][5]  = { "0000", "F000", "0F00", "FF00",
                                             "00F0", "F0F0", "0FF0", "FFF0",
                                             "000F", "F00F", "0F0F", "FF0F",
                                             "00FF", "F0FF", "0FFF", "FFFF"
                                           };
const char RCSwitch::TYPE_D_CODE[5][2][9]  = { { "11100001", "11110000" },
                                               { "00000000", "00010001" },
                                               { "10000010", "10010011" },
                                               { "11000011", "11010010" },
                                               { "01000001", "01010000" }
                                             };
const int  RCSwitch::PULSE_LENGTH[MAX_PROTOCOLS] = { 0,
                                                     350, // Type A
                                                     650,
                                                     100,
                                                     666, // Type D
                                                   };
const int  RCSwitch::REPEAT_TRANSMIT[MAX_PROTOCOLS] = { 0,
                                                        10, // Type A
                                                        10,
                                                        10,
                                                        4, // Type D
                                                      };
const int  RCSwitch::SYNC_FACTOR[MAX_PROTOCOLS]        = { 0, // Dummy
                                                           PROTOCOL_A_SYNC_FACTOR,
                                                           PROTOCOL_B_SYNC_FACTOR,
                                                           PROTOCOL_C_SYNC_FACTOR,
                                                           PROTOCOL_D_SYNC_FACTOR,
                                                         };
const int  RCSwitch::ZERO_FIRST_CYCLES[MAX_PROTOCOLS]  = { 0, // Dummy
                                                           PROTOCOL_A_ZERO_FIRST_CYCLES,
                                                           PROTOCOL_B_ZERO_FIRST_CYCLES,
                                                           PROTOCOL_C_ZERO_FIRST_CYCLES,
                                                           PROTOCOL_D_ZERO_FIRST_CYCLES,
                                                         };
const int  RCSwitch::ZERO_SECOND_CYCLES[MAX_PROTOCOLS] = { 0, // Dummy
                                                           PROTOCOL_A_ZERO_SECOND_CYCLES,
                                                           PROTOCOL_B_ZERO_SECOND_CYCLES,
                                                           PROTOCOL_C_ZERO_SECOND_CYCLES,
                                                           PROTOCOL_D_ZERO_SECOND_CYCLES,
                                                         };
const int  RCSwitch::ONE_FIRST_CYCLES[MAX_PROTOCOLS]  = { 0, // Dummy
                                                          PROTOCOL_A_ONE_FIRST_CYCLES,
                                                          PROTOCOL_B_ONE_FIRST_CYCLES,
                                                          PROTOCOL_C_ONE_FIRST_CYCLES,
                                                          PROTOCOL_D_ONE_FIRST_CYCLES,
                                                        };
const int  RCSwitch::ONE_SECOND_CYCLES[MAX_PROTOCOLS] = { 0, // Dummy
                                                          PROTOCOL_A_ONE_SECOND_CYCLES,
                                                          PROTOCOL_B_ONE_SECOND_CYCLES,
                                                          PROTOCOL_C_ONE_SECOND_CYCLES,
                                                          PROTOCOL_D_ONE_SECOND_CYCLES,
                                                        };
const bool RCSwitch::HIGH_FIRST[MAX_PROTOCOLS]        = { 0, // Dummy
                                                          PROTOCOL_A_HIGH_FIRST,
                                                          PROTOCOL_B_HIGH_FIRST,
                                                          PROTOCOL_C_HIGH_FIRST,
                                                          PROTOCOL_D_HIGH_FIRST,
                                                        };
RCSwitch::RCSwitch()
  : m_nTransmitterPin(-1)
  , m_bAutoChangeMode(false)
  , m_backup_eProtocol(TYPE_UNDEF)
  , m_backup_nPulseLength(0)
  , m_backup_nRepeatTransmit(0)
#ifndef RCSwitchDisableReceiving
  , m_nReceiverInterrupt(-1)
#endif // RCSwitchDisableReceiving
{
  setRepeatTransmit(10);
  setProtocol(TYPE_A);
#ifndef RCSwitchDisableReceiving
  setReceiveTolerance(60);
  RCSwitch::s_nReceivedValue = 0;
#endif // RCSwitchDisableReceiving
}

/**
 * Deprecated, use toggle(char *sGroup, char *sSwitch, bool bStatus) instead!
 * Toggle a remote switch  (Type A with 10 pole DIP switches)
 *
 * @param sGroup         Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nSwitchNumber  Number of the switch itself (1..5)
 * @param bStatus        Status to toggle to
 */
void RCSwitch::toggleTypeA(char *sGroup, int nSwitchNumber, bool bStatus)
{
  if (nSwitchNumber < 0 || nSwitchNumber > 5)
  {
    return;
  }
  toggleTypeA(sGroup, (char*) TYPE_A_CODE[nSwitchNumber], bStatus);
}

/**
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitch::toggleTypeA(char *sGroup, char *sSwitch, bool bStatus)
{
  char *str = getCodeWordA(sGroup, sSwitch, bStatus);
  save(TYPE_A);
  sendTriState( str );
  load();
  delete str;
}

/**
 * Switch a remote switch on (Type B with two rotary/sliding switches)
 *
 * @param nGroupNumber   Number of the switch group (1..4)
 * @param nSwitchNumber  Number of the switch itself (1..4)
 * @param bStatus        Status to toggle to
 */
void RCSwitch::toggleTypeB(int nGroupNumber, int nSwitchNumber, bool bStatus)
{
  char *str = getCodeWordB(nGroupNumber, nSwitchNumber, bStatus);
  save(TYPE_B);
  sendTriState( str );
  load();
  delete str;
}

/**
 * Toggle a remote switch (Type C Intertechno)
 *
 * TODO: Family Code a..f or 0..f?
 * @param cFamily        Familycode (a..f)
 * @param nGroupNumber   Number of group (1..4)
 * @param nSwitchNumber  Number of switch (1..4)
 * @param bStatus        Status to toggle to
 */
void RCSwitch::toggleTypeC(char cFamily, int nGroupNumber, int nSwitchNumber, bool bStatus)
{
  char *str = getCodeWordC(cFamily, nGroupNumber, nSwitchNumber, bStatus);
  save(TYPE_C);
  sendTriState( str );
  load();
  delete str;
}

void RCSwitch::toggleTypeD(char *cGroup, int nSwitchNumber, bool bStatus)
{
  char *str = getCodeWordD(cGroup, nSwitchNumber, bStatus);
  save(TYPE_D);
  send( str );
  load();
  delete str;
}

/**
 * Toggle a remote switch (Type E REV)
 *
 * @param cGroup        Code of the switch group (A,B,C,D)
 * @param nSwitchNumber Number of the switch itself (1..3)
 * @param bStatus       Status to toggle to
 */
void RCSwitch::toggleTypeE(char cGroup, int nSwitchNumber, bool bStatus)
{
  char *str = getCodeWordE(cGroup, nSwitchNumber, bStatus);
  save(TYPE_E);
  sendTriState( str );
  load();
  delete str;
}

/**
  * Sets the protocol to send with optional pulse length in microseconds.
  * If pulse length is not given, the Protocol's default is taken
  */
void RCSwitch::setProtocol(eProtocol _eProtocol, int nPulseLength, int nRepeatTransmit)
{
  if (_eProtocol < TYPE_MINIMUM ||
      _eProtocol > TYPE_MAXIMUM
     )
  {
    return;
  }

  m_eProtocol = _eProtocol;

  if (nPulseLength > 0)
  {
    setPulseLength(nPulseLength);
  }
  else
  {
    setPulseLength(PULSE_LENGTH[_eProtocol]);
  }

  if (nRepeatTransmit > 0)
  {
    setRepeatTransmit(nRepeatTransmit);
  }
  else
  {
    setRepeatTransmit(REPEAT_TRANSMIT[_eProtocol]);
  }
}

/**
  * Sets pulse length in microseconds
  */
void RCSwitch::setPulseLength(int nPulseLength)
{
  m_nPulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void RCSwitch::setRepeatTransmit(int nRepeatTransmit)
{
  m_nRepeatTransmit = nRepeatTransmit;
}

/**
 * Set Receiving Tolerance
 */
#ifndef RCSwitchDisableReceiving
void RCSwitch::setReceiveTolerance(int nPercent)
{
  RCSwitch::s_nReceiveTolerance = nPercent;
}
#endif // RCSwitchDisableReceiving


/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RCSwitch::enableTransmit(int nTransmitterPin)
{
  m_nTransmitterPin = nTransmitterPin;
  pinMode(m_nTransmitterPin, OUTPUT);
}

/**
  * Disable transmissions
  */
void RCSwitch::disableTransmit()
{
  m_nTransmitterPin = -1;
}

/**
 * Returns a char[13], representing the Code Word to be send.
 *
 * getCodeWordA(char*, char*)
 *
 */
char* RCSwitch::getCodeWordA(char* sGroup, char* sDevice, bool bOn)
{
  char* sReturn    = new char[13];
  int   nReturnPos = 0;

  for (int i=0; i<5; ++i)
  {
    sReturn[nReturnPos++] = (sGroup[i] == '0') ? 'F' : '0';
  }

  for (int i=0; i<5; ++i)
  {
    sReturn[nReturnPos++] = (sDevice[i] == '0') ? 'F' : '0';
  }

  sReturn[nReturnPos++] = bOn ? '0' : 'F';
  sReturn[nReturnPos++] = bOn ? 'F' : '0';
  sReturn[nReturnPos] = '\0';

  return sReturn;
}

/**
 * Returns a char[13], representing the Code Word to be send.
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
 * @return char[13]
 */
char* RCSwitch::getCodeWordB(int nAddressCode, int nChannelCode, bool bStatus)
{
  if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4) {
    return '\0';
  }

  int   nReturnPos = 0;
  char* sReturn    = new char[13];

  for (int i=0; i<4; ++i) {
    sReturn[nReturnPos++] = TYPE_B_CODE[nAddressCode][i];
  }

  for (int i=0; i<4; ++i) {
    sReturn[nReturnPos++] = TYPE_B_CODE[nChannelCode][i];
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
char* RCSwitch::getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus)
{
  // Whoever know about Intertechno:
  // TODO: If sFamily actually is 0..f, sFamily has to be matched correctly
  //       distinguished between 0..9 plus a..f!
  // However: This might not work, although I didn't change functionallity
  // Malte Diers, 22.11.2013

  if (sFamily < 'a')
  {
    // To also enable capital 'A' to 'F'
    sFamily += 32;
  }
  if (    sFamily < 'a' || sFamily > 'f'
       || nGroup  < 1   || nGroup  > 4
       || nDevice < 1   || nDevice > 4
     )
  {
    return '\0';
  }

  char* sReturn    = new char[13];
  int   nReturnPos = 0;

  for (int i=0; i<4; ++i)
  {
    sReturn[nReturnPos++] = TYPE_C_CODE[ sFamily - 'a' ][i];
  }

  char* sDeviceGroupCode = dec2bin( (nDevice-1) + (nGroup-1)*4, 4  );

  for (int i=0; i<4; ++i)
  {
    sReturn[nReturnPos++] = (sDeviceGroupCode[3-i] == '1' ? 'F' : '0');
  }

  delete sDeviceGroupCode;

  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = bStatus ? 'F' : '0';
  sReturn[nReturnPos]   = '\0';
  return sReturn;
}

/**
 * Decoding for the Quigg Switch Type
 *
 * Returns a char[22], representing the States to be send.
 * A Code Word consists of 1 start bit, 12 address bits and 8 command data bits.
 * A Code Bit can have 2 different states: "0" (low), "1" (high)
 *
 * +--------------+--------------------------------+------------------------------+
 * | 1 bits start | 12 bits address (device group) | 8 bits (command/switch data) |
 * |        1     |           110011001100         |             00010001         |
 * +--------------+--------------------------------+------------------------------+
 *
 * Source: https://github.com/d-a-n/433-codes/blob/master/database.md#quigg
 *
 * @param sGroup        12-bit Binary ID of the Device Group
 * @param nDevice       Number of the switch itself (1..4, or 0 to switch the entire Group)
 * @param bStatus       Wether to switch on (true) or off (false)
 *
 * @return char[22]
 */

char *RCSwitch::getCodeWordD(char *sGroup, int nDevice, bool bStatus)
{
  char* sReturn    = new char[22];
  int   nReturnPos = 0;

  // Startbit
  sReturn[nReturnPos++] = '1';

  // 12 bit Group
  for (int i=0; i<12; ++i)
  {
    sReturn[nReturnPos++] = sGroup[i];
  }

  // 8 Bit Device Identifier + Status (undividable!)
  for (int i=0; i<8; ++i)
  {
    sReturn[nReturnPos++] = TYPE_D_CODE[nDevice][bStatus][i];
  }
  sReturn[nReturnPos] = 0;

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
 * @param bStatus       Wether to switch on (true) or off (false)
 *
 * @return char[13]
 */

char* RCSwitch::getCodeWordE(char sGroup, int nDevice, bool bStatus)
{
  // Building 4 bits address
  char *sGroupCode = 0;
  switch(sGroup){
    case 'a':
    case 'A':
      sGroupCode = dec2bin(8, 4, 'F');
      break;
    case 'b':
    case 'B':
      sGroupCode = dec2bin(4, 4, 'F');
      break;
    case 'c':
    case 'C':
      sGroupCode = dec2bin(2, 4, 'F');
      break;
    case 'd':
    case 'D':
      sGroupCode = dec2bin(1, 4, 'F');
      break;
    default:
      return '\0';
  }

  char* sReturn    = new char[13];
  int   nReturnPos = 0;

  for (int i=0; i<4; ++i)
  {
    sReturn[nReturnPos++] = sGroupCode[i];
  }

  // No more need for this...
  delete sGroupCode;


  // Building 3 bits address
  char *sDevice = 0;
  switch(nDevice) {
    case 1:
      sDevice = dec2bin(4, 3, 'F');
      break;
    case 2:
      sDevice = dec2bin(2, 3, 'F');
      break;
    case 3:
      sDevice = dec2bin(1, 3, 'F');
      break;
    default:
      delete sReturn;
      return '\0';
  }

  for (int i=0; i<3; ++i)
  {
    sReturn[nReturnPos++] = sDevice[i];
  }
  // No more need for this...
  delete sDevice;

  // Some space left...
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';

  // encode on or off
  sReturn[nReturnPos++] = bStatus ? '1' : '0';
  sReturn[nReturnPos++] = bStatus ? '0' : '1';

  // last position terminate string
  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * @param sCodeWord   /^[10FS]*$/  -> see getCodeWord
 */
void RCSwitch::send(char* sCodeWord, bool tristate)
{
  for (int nRepeat=0; nRepeat<m_nRepeatTransmit; ++nRepeat) {
    int i = 0;
    while (sCodeWord[i] != '\0') {
      switch(sCodeWord[i]) {
        case '0':
          tristate ? sendT0() : send0();
          break;
        case '1':
          tristate ? sendT1() : send1();
          break;
        case 'F':
        case 'f':
          if (tristate)
          {
            sendTF();
          }
          break;
        default:
          break;
      }
      ++i;
    }
    sendSync();
  }
}

void RCSwitch::send(unsigned long code, unsigned int length)
{
  char *str = dec2bin(code, length);
  send( str );
  delete str;
}

void RCSwitch::transmit(int nFirstPulses, int nSecondPulses, bool bHighFirst)
{
#ifndef RCSwitchDisableReceiving
  bool disabled_Receive          = false;
  int  nReceiverInterrupt_backup = m_nReceiverInterrupt;
#endif // RCSwitchDisableReceiving
  if (m_nTransmitterPin != -1) {
#ifndef RCSwitchDisableReceiving
    if (m_nReceiverInterrupt != -1) {
      disableReceive();
      disabled_Receive = true;
    }
#endif // RCSwitchDisableReceiving
    digitalWrite(m_nTransmitterPin, bHighFirst ? HIGH : LOW);
    delayMicroseconds( m_nPulseLength * nFirstPulses);
    digitalWrite(m_nTransmitterPin, bHighFirst ? LOW : HIGH);
    delayMicroseconds( m_nPulseLength * nSecondPulses);

#ifndef RCSwitchDisableReceiving
    if(disabled_Receive){
      enableReceive(nReceiverInterrupt_backup);
    }
#endif // RCSwitchDisableReceiving
  }
}
/**
 * Sends a "0" Bit
 *                       _
 * Waveform Protocol A: | |___
 *                       _
 * Waveform Protocol B: | |__
 *                       ____
 * Waveform Protocol C: |    |___________
 *                        __
 * Waveform Protocol D: _|  |
 */
void RCSwitch::send0()
{
  transmit(ZERO_FIRST_CYCLES[m_eProtocol], ZERO_SECOND_CYCLES[m_eProtocol], HIGH_FIRST[m_eProtocol]);
/* DEPRECATED:
  switch(m_eProtocol)
  {
    case TYPE_A:
      transmit(1,3);
      break;
    case TYPE_B:
      transmit(1,2);
      break;
    case TYPE_C:
      transmit(4,11);
      break;
    case TYPE_D:
      transmit(1,2,false);
      break;
    default:
      break;
  }
  */
}

/**
 * Sends a "1" Bit
 *                       ___
 * Waveform Protocol A: |   |_
 *                       __
 * Waveform Protocol B: |  |_
 *                       _________
 * Waveform Protocol C: |         |______
 *                         _
 * Waveform Protocol D: __| |
 */
void RCSwitch::send1()
{
  transmit(ONE_FIRST_CYCLES[m_eProtocol], ONE_SECOND_CYCLES[m_eProtocol], HIGH_FIRST[m_eProtocol]);
/* DEPRECATED:
  switch(m_eProtocol)
  {
    case TYPE_A:
      transmit(3,1);
      break;
    case TYPE_B:
      transmit(2,1);
      break;
    case TYPE_C:
      transmit(9,6);
      break;
    case TYPE_D:
      transmit(2,1,false);
      break;
    default:
      break;
  }
  */
}


/**
 * Sends a Tri-State "0" Bit
 *            _     _
 * Waveform: | |___| |___
 */
void RCSwitch::sendT0()
{
  transmit(1,3);
  transmit(1,3);
}

/**
 * Sends a Tri-State "1" Bit
 *            ___   ___
 * Waveform: |   |_|   |_
 */
void RCSwitch::sendT1()
{
  transmit(3,1);
  transmit(3,1);
}

/**
 * Sends a Tri-State "F" Bit
 *            _     ___
 * Waveform: | |___|   |_
 */
void RCSwitch::sendTF()
{
  transmit(1,3);
  transmit(3,1);
}

/**
 * Sends a "Sync" Bit
 *                       _
 * Waveform Protocol A: | |_______________________________
 *                       _
 * Waveform Protocol B: | |__________
 *                       ____
 * Waveform Protocol C: |    |_______________________________________________________________________
 *
 * Waveform Protocol D: (none, just pause 80 msecs)
 */
void RCSwitch::sendSync()
{
  // Note: Too specific to put this into constants...
  switch(m_eProtocol)
  {
    case TYPE_A:
      transmit(1,31);
      break;
    case TYPE_B:
      transmit(1,10);
      break;
    case TYPE_C:
      transmit(4,71);
      break;
    case TYPE_D:
      transmit(0,1);
      delayMicroseconds(80000);
      break;
    default:
      break;
  }
}

bool RCSwitch::autoChangeMode() const
{
  return m_bAutoChangeMode;
}

void RCSwitch::setAutoChangeMode(bool bAutoChangeMode)
{
  m_bAutoChangeMode = bAutoChangeMode;
}


#ifndef RCSwitchDisableReceiving
/**
 * Enable receiving data
 */
void RCSwitch::enableReceive(int interrupt)
{
  m_nReceiverInterrupt = interrupt;
  enableReceive();
}

void RCSwitch::enableReceive()
{
  // TODO: Mayby better "if (foo<0) return;" ? What about foo == -2?
  if (m_nReceiverInterrupt != -1)
  {
    RCSwitch::s_nReceivedValue     = 0;
    RCSwitch::s_nReceivedBitlength = 0;
    attachInterrupt(m_nReceiverInterrupt, handleInterrupt, CHANGE);
  }
}

/**
 * Disable receiving data
 */
void RCSwitch::disableReceive()
{
  detachInterrupt(m_nReceiverInterrupt);
  m_nReceiverInterrupt = -1;
}

bool RCSwitch::available() const
{
  return (RCSwitch::s_nReceivedValue != 0);
}

void RCSwitch::resetAvailable()
{
  RCSwitch::s_nReceivedValue = 0;
}

unsigned long RCSwitch::getReceivedValue() const
{
  return RCSwitch::s_nReceivedValue;
}

unsigned int RCSwitch::getReceivedBitlength() const
{
  return RCSwitch::s_nReceivedBitlength;
}

unsigned int RCSwitch::getReceivedDelay() const
{
  return RCSwitch::s_nReceivedDelay;
}

RCSwitch::eProtocol RCSwitch::getReceivedProtocol() const
{
  return RCSwitch::s_nReceivedProtocol;
}

unsigned int* RCSwitch::getReceivedRawdata() const
{
  return RCSwitch::timings;
}

bool RCSwitch::receiveProtocol(eProtocol prot, unsigned int changeCount)
{
  if (prot < TYPE_MINIMUM ||
      prot > TYPE_MAXIMUM
     )
  {
    return false;
  }
  unsigned long code = 0;
  unsigned long delay = RCSwitch::timings[0] / SYNC_FACTOR[prot];
  unsigned long delayTolerance = delay * RCSwitch::s_nReceiveTolerance * 0.01;

  for (unsigned int i=1; i<changeCount ; i+=2)
  {
    if  (   RCSwitch::timings[i]   > delay*ZERO_FIRST_CYCLES[prot] - delayTolerance
         && RCSwitch::timings[i]   < delay*ZERO_FIRST_CYCLES[prot] + delayTolerance
         && RCSwitch::timings[i+1] > delay*ZERO_SECOND_CYCLES[prot]  - delayTolerance
         && RCSwitch::timings[i+1] < delay*ZERO_SECOND_CYCLES[prot]  + delayTolerance)
    {
      code = code << 1;
    }
    else if (   RCSwitch::timings[i]   > delay*ONE_FIRST_CYCLES[prot] - delayTolerance
             && RCSwitch::timings[i]   < delay*ONE_FIRST_CYCLES[prot] + delayTolerance
             && RCSwitch::timings[i+1] > delay*ONE_SECOND_CYCLES[prot]  - delayTolerance
             && RCSwitch::timings[i+1] < delay*ONE_SECOND_CYCLES[prot]  + delayTolerance)
    {
      code+=1;
      code = code << 1;
    }
    else
    {
      // Failed
      i = changeCount;
      code = 0;
    }
  }
  code = code >> 1;
  if (changeCount > 6)   // ignore < 4bit values as there are no devices sending 4bit values => noise
  {
    RCSwitch::s_nReceivedValue     = code;
    RCSwitch::s_nReceivedBitlength = changeCount / 2;
    RCSwitch::s_nReceivedDelay     = delay;
    RCSwitch::s_nReceivedProtocol  = prot;
  }
  return (code != 0);
}

void RCSwitch::handleInterrupt()
{
  static unsigned int  duration;
  static unsigned int  changeCount;
  static unsigned long lastTime;
  static unsigned int  repeatCount;


  long time = micros();
  duration = time - lastTime;

  if (duration > 5000 &&
      duration > (RCSwitch::timings[0] - 200) &&
      duration < (RCSwitch::timings[0] + 200)
     )
  {
    repeatCount++;
    changeCount--;
    if (repeatCount == 2)
    {
      if (receiveProtocol(TYPE_A, changeCount) == false)
      {
        if (receiveProtocol(TYPE_B, changeCount) == false)
        {
          if (receiveProtocol(TYPE_C, changeCount) == false)
          {
            // TODO: Maybe better not try Protocol D...?!?
            if (receiveProtocol(TYPE_D, changeCount) == false){
              // TODO: Someone said
                  //failed
              // So: What to to here?
            }
          }
        }
      }
      repeatCount = 0;
    }
    changeCount = 0;
  }
  else if (duration > 5000)
  {
    changeCount = 0;
  }

  if (changeCount >= RCSWITCH_MAX_CHANGES)
  {
    changeCount = 0;
    repeatCount = 0;
  }
  RCSwitch::timings[changeCount++] = duration;
  lastTime = time;
}
#endif // RCSwitchDisableReceiving

//char* RCSwitch::dec2binWcharfill(unsigned long dec, unsigned int bitLength, char fill)
//{
//  // Note: sizeof(long) is not necessarily 32bit!
//  char         *bin = new char[64];
//  unsigned int  i   = 0;

//  while (dec > 0) {
//    bin[32+i++] = ((dec & 1) > 0) ? '1' : fill;
//    dec = dec >> 1;
//  }

//  for (unsigned int j = 0; j< bitLength; j++) {
//    if (j >= bitLength - i) {
//      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
//    }else {
//      bin[j] = fill;
//    }
//  }
//  bin[bitLength] = '\0';

//  return bin;
//}

// Note: Some easier algorithm
char* RCSwitch::dec2bin(unsigned long dec, unsigned int bitLength, char fill)
{
  char *bin = new char[bitLength + 1];

  bin[bitLength] = 0;
  for (int i=bitLength-1; i>=0; --i)
  {
    bin[i] = (dec & 1) > 0 ? '1' : fill;
    dec = dec >> 1;
  }
  // TODO: You could possibly log an error if dec still is > 0,
  // what implies that bitlength is too small...
  return bin;
}

void RCSwitch::save(eProtocol prot)
{
  if (!m_bAutoChangeMode)
    return;

  m_backup_eProtocol       = m_eProtocol;
  m_backup_nPulseLength    = m_nPulseLength;
  m_backup_nRepeatTransmit = m_nRepeatTransmit;

  setProtocol(prot);
}

void RCSwitch::load()
{
  if(!m_bAutoChangeMode)
    return;

  setProtocol(m_backup_eProtocol, m_backup_nPulseLength, m_backup_nRepeatTransmit);
}

