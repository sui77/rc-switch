/*
  RCSwitch - Arduino/RPi libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  For RaspberryPi Compiling, please use command
  g++ -DRASPBERRY -lwiringPi <files>

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com

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
#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific
  #include "Energia.h"
#elif defined(RASPBERRY) // RaspberryPi with WiringPi
  #ifdef __cplusplus
    extern "C" {
  #endif
    #include <wiringPi.h>
  #ifdef __cplusplus
  }
  #endif
#else
    #include "WProgram.h"
#endif // ARDUINO >= 100


// At least for the ATTiny X4/X5, receiving has to be disabled due to
// missing libm depencies (udivmodhi4)
#if defined( __AVR_ATtinyX5__ ) or defined ( __AVR_ATtinyX4__ ) or defined ( RASPBERRY )
#define RCSwitchDisableReceiving
#endif // __AVR_ATtinyX4/X5___


// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES           67

// i.e. ProtocolCount + 1 (for TYPE_UNDEF)
#define MAX_PROTOCOLS                   5

#define PROTOCOL_A_SYNC_FACTOR         31
#define PROTOCOL_A_ZERO_FIRST_CYCLES    1
#define PROTOCOL_A_ZERO_SECOND_CYCLES   3
#define PROTOCOL_A_ONE_FIRST_CYCLES     3
#define PROTOCOL_A_ONE_SECOND_CYCLES    1
#define PROTOCOL_A_HIGH_FIRST        true

#define PROTOCOL_B_SYNC_FACTOR         10
#define PROTOCOL_B_ZERO_FIRST_CYCLES    1
#define PROTOCOL_B_ZERO_SECOND_CYCLES   2
#define PROTOCOL_B_ONE_FIRST_CYCLES     2
#define PROTOCOL_B_ONE_SECOND_CYCLES    1
#define PROTOCOL_B_HIGH_FIRST        true

#define PROTOCOL_C_SYNC_FACTOR         71
#define PROTOCOL_C_ZERO_FIRST_CYCLES    4
#define PROTOCOL_C_ZERO_SECOND_CYCLES  11
#define PROTOCOL_C_ONE_FIRST_CYCLES     9
#define PROTOCOL_C_ONE_SECOND_CYCLES    6
#define PROTOCOL_C_HIGH_FIRST        true

// I think, this will work for receive, however, I haven't tested, as I don't own a receiver...
// As Type D doesn't sync acc. to https://github.com/d-a-n/433-codes/blob/master/database.md#quigg
// the sync factor is totally undetermined.
// Malte Diers, 22.11.2013
#define PROTOCOL_D_SYNC_FACTOR          1
#define PROTOCOL_D_ZERO_FIRST_CYCLES    1
#define PROTOCOL_D_ZERO_SECOND_CYCLES   2
#define PROTOCOL_D_ONE_FIRST_CYCLES     2
#define PROTOCOL_D_ONE_SECOND_CYCLES    1
#define PROTOCOL_D_HIGH_FIRST       false

class RCSwitch {

  public:
    RCSwitch();

    enum eProtocol {
      TYPE_UNDEF = 0,
      TYPE_MINIMUM = TYPE_UNDEF,
      TYPE_A = 1,
      TYPE_B = 2,
      TYPE_C = 3,
      TYPE_D = 4,
      TYPE_E = 3,  // TODO: Which Protocol does REV use?
      TYPE_MAXIMUM = TYPE_D
    };

    // Deprecated BEGIN
    void toggleTypeA(               char* sGroup,       int   nSwitchNumber, bool bStatus);
    // Deprecated END
    void toggleTypeA(               char* sGroup,       char* sSwitch,       bool bStatus);
    void toggleTypeB(               int   nGroupNumber, int   nSwitchNumber, bool bStatus);
    void toggleTypeC(char  cFamily, int   nGroupNumber, int   nSwitchNumber, bool bStatus);
    void toggleTypeD(               char* cGroup,       int   nSwitchNumber, bool bStatus);
    void toggleTypeE(               char  cGroup,       int   nSwitchNumber, bool bStatus);

    // Deprecated BEGIN
    void switchOn(int nGroupNumber, int nSwitchNumber) { toggleTypeB(nGroupNumber, nSwitchNumber, true); }
    void switchOff(int nGroupNumber, int nSwitchNumber){ toggleTypeB(nGroupNumber, nSwitchNumber, false); }
    void switchOn(char* sGroup, int nSwitchNumber){ toggleTypeA(sGroup, nSwitchNumber, true); }
    void switchOff(char* sGroup, int nSwitchNumber){ toggleTypeA(sGroup, nSwitchNumber, false); }
    void switchOn(char sFamily, int nGroupNumber, int nSwitchNumber){ toggleTypeC(sFamily, nGroupNumber, nSwitchNumber, true); }
    void switchOff(char sFamily, int nGroupNumber, int nSwitchNumber){ toggleTypeC(sFamily, nGroupNumber, nSwitchNumber, false); }
    void switchOn(char* sGroup, char* nSwitchNumber){ toggleTypeA(sGroup, nSwitchNumber, true); }
    void switchOff(char* sGroup, char* nSwitchNumber){ toggleTypeA(sGroup, nSwitchNumber, false); }
    void switchOn(char sGroup, int nSwitchNumber){ toggleTypeE(sGroup, nSwitchNumber, true); }
    void switchOff(char sGroup, int nSwitchNumber){ toggleTypeE(sGroup, nSwitchNumber, false); }
    // Deprecated END

    void send(unsigned long code, unsigned int length);
    void send(char* sCodeWord, bool tristate = false);
    // Deprecated BEGIN
    void sendTriState(char* code) {send(code, true);}
    // Deprecated END

#ifndef RCSwitchDisableReceiving
    void enableReceive(int interrupt);
    void enableReceive();
    void disableReceive();
    bool available() const;
    void resetAvailable();

    unsigned long getReceivedValue() const;
    unsigned int  getReceivedBitlength() const;
    unsigned int  getReceivedDelay() const;
    eProtocol     getReceivedProtocol() const;
    unsigned int* getReceivedRawdata() const;
#endif // RCSwitchDisableReceiving

    void enableTransmit(int nTransmitterPin);
    void disableTransmit();

    void setProtocol(eProtocol _eProtocol, int nPulseLength = 0, int nRepeatTransmit = 0);
    void setPulseLength(int nPulseLength);
    void setRepeatTransmit(int nRepeatTransmit);

#ifndef RCSwitchDisableReceiving
    void setReceiveTolerance(int nPercent);
#endif // RCSwitchDisableReceiving

    bool autoChangeMode() const;
    void setAutoChangeMode(bool bAutoChangeMode);

private:
    static char* getCodeWordA(char* sGroup, char* sDevice, bool bStatus);
    static char* getCodeWordB(int nGroupNumber, int nSwitchNumber, bool bStatus);
    static char* getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus);
    static char* getCodeWordD(char* group, int nDevice, bool bStatus);
    static char* getCodeWordE(char group, int nDevice, bool bStatus);

    void sendT0();
    void sendT1();
    void sendTF();

    void send0();
    void send1();

    void sendSync();

    void transmit(int nFirstPulses, int nSecondPulses, bool bHighFirst = true);

    static char* dec2bin(unsigned long dec, unsigned int length, char fill = '0');

    void save(eProtocol prot);
    void load();

#ifndef RCSwitchDisableReceiving
    static void handleInterrupt();
    static bool receiveProtocol(eProtocol pro, unsigned int changeCount);
#endif // RCSwitchDisableReceiving

    int       m_nTransmitterPin;
    bool      m_bAutoChangeMode;

    eProtocol m_eProtocol;
    int       m_nPulseLength;
    int       m_nRepeatTransmit;

    eProtocol m_backup_eProtocol;
    int       m_backup_nPulseLength;
    int       m_backup_nRepeatTransmit;

#ifndef RCSwitchDisableReceiving
                    int  m_nReceiverInterrupt;
    static          int  s_nReceiveTolerance;
    static unsigned long s_nReceivedValue;
    static unsigned int  s_nReceivedBitlength;
    static unsigned int  s_nReceivedDelay;
    static     eProtocol s_nReceivedProtocol;
    /*
     * timings[0] contains sync timing, followed by a number of bits
     */
    static unsigned int timings[RCSWITCH_MAX_CHANGES];
#endif // RCSwitchDisableReceiving

    static const char TYPE_A_CODE[ 6][6];
    static const char TYPE_B_CODE[ 5][5];
    static const char TYPE_C_CODE[16][5];
    static const char TYPE_D_CODE[ 5][2][9];
    static const int  SYNC_FACTOR[MAX_PROTOCOLS];
    static const int  ZERO_FIRST_CYCLES[MAX_PROTOCOLS];
    static const int  ZERO_SECOND_CYCLES[MAX_PROTOCOLS];
    static const int  ONE_FIRST_CYCLES[MAX_PROTOCOLS];
    static const int  ONE_SECOND_CYCLES[MAX_PROTOCOLS];
    static const bool HIGH_FIRST[MAX_PROTOCOLS];
    static const int  PULSE_LENGTH[MAX_PROTOCOLS];
    static const int  REPEAT_TRANSMIT[MAX_PROTOCOLS];
};
