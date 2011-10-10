/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  Contributors:
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  
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
#ifndef _RCSwitch_h
#define _RCSwitch_h

#include "WProgram.h"

// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES 67

typedef void (*RCSwitchCallback)(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw);

class RCSwitch {

  public:
    RCSwitch();
  
    RCSwitch(int nPin);                // deprecated
    RCSwitch(int nPin, int nDelay);    // deprecated
    
    void switchOn(int nGroupNumber, int nSwitchNumber);
    void switchOff(int nGroupNumber, int nSwitchNumber);
    void switchOn(char* sGroup, int nSwitchNumber);
    void switchOff(char* sGroup, int nSwitchNumber);
    void switchOn(char sFamily, int nGroup, int nDevice);
    void switchOff(char sFamily, int nGroup, int nDevice);
    

    void sendTriState(char* Code);
    void send(unsigned long Code, unsigned int length);
    void send(char* Code);

    void sendQ(unsigned long Code, unsigned int length);
    void sendQ(char* Code);

    
    void enableReceive(int interrupt, RCSwitchCallback callback);
    void disableReceive();
  
    void enableTransmit(int nTransmitterPin);
    void disableTransmit();
    void setPulseLength(int nPulseLength);
    void setRepeatTransmit(int RepeatTransmit);
  
  private:
    char* getCodeWordB(int nGroupNumber, int nSwitchNumber, boolean bStatus);
    char* getCodeWordA(char* sGroup, int nSwitchNumber, boolean bStatus);
    char* getCodeWordC(char sFamily, int nGroup, int nDevice, boolean bStatus);
    void sendT0();
    void sendT1();
    void sendTF();
    void send0();
    void send1();
    void sendSync();
    void send0Q();
    void send1Q();
    void sendSyncQ();    
    void transmitHL(int nHighPulses, int nLowPulses);
    void transmitLH(int nLowPulses, int nHighPulses);

    static char* dec2binWzerofill(unsigned long dec, unsigned int length);
    
    static void receiveInterrupt();
    static RCSwitchCallback mCallback;
    int nReceiverInterrupt;
    int nTransmitterPin;
    int nPulseLength;

    int RepeatTransmit;
    
};

#endif