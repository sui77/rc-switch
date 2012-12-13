/*
  RCSwitch - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.

  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Dominik Fischer / dom_fischer(at)web(dot)de
  
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

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

// Number of maximum High/Low changes per packet.
// We can handle up to (unsigned long) => 32 bit * 2 H/L changes per bit + 2 for sync
#define RCSWITCH_MAX_CHANGES 67


class RCSwitch {

  public:
    RCSwitch();
    
    void switchOn(int nGroupNumber, int nSwitchNumber);
    void switchOff(int nGroupNumber, int nSwitchNumber);
    void switchOn(char* sGroup, int nSwitchNumber);
    void switchOff(char* sGroup, int nSwitchNumber);
    void switchOn(char sFamily, int nGroup, int nDevice);
    void switchOff(char sFamily, int nGroup, int nDevice);
	void switchOn(char* sGroup, char* sDevice);
	void switchOff(char* sGroup, char* sDevice);

    void sendTriState(char* Code);
    void send(unsigned long Code, unsigned int length);
    void send(char* Code);
    
    void enableReceive(int interrupt);
    void enableReceive();
    void disableReceive();
    bool available();
	void resetAvailable();
	
    unsigned long getReceivedValue();
    unsigned int getReceivedBitlength();
    unsigned int getReceivedDelay();
	unsigned int getReceivedProtocol();
    unsigned int* getReceivedRawdata();
  
    void enableTransmit(int nTransmitterPin);
    void disableTransmit();
    void setPulseLength(int nPulseLength);
    void setRepeatTransmit(int nRepeatTransmit);
    void setReceiveTolerance(int nPercent);
	void setProtocol(int nProtocol);
	void setProtocol(int nProtocol, int nPulseLength);
  
  private:
    char* getCodeWordB(int nGroupNumber, int nSwitchNumber, boolean bStatus);
    char* getCodeWordA(char* sGroup, int nSwitchNumber, boolean bStatus);
    char* getCodeWordA(char* sGroup, char* sDevice, boolean bStatus);
    char* getCodeWordC(char sFamily, int nGroup, int nDevice, boolean bStatus);
    void sendT0();
    void sendT1();
    void sendTF();
    void send0();
    void send1();
    void sendSync();
    void transmit(int nHighPulses, int nLowPulses);

    static char* dec2binWzerofill(unsigned long dec, unsigned int length);
    
    static void handleInterrupt();
	static bool receiveProtocol1(unsigned int changeCount);
	static bool receiveProtocol2(unsigned int changeCount);
    int nReceiverInterrupt;
    int nTransmitterPin;
    int nPulseLength;
    int nRepeatTransmit;
	char nProtocol;

	static int nReceiveTolerance;
    static unsigned long nReceivedValue;
    static unsigned int nReceivedBitlength;
	static unsigned int nReceivedDelay;
	static unsigned int nReceivedProtocol;
    static unsigned int timings[RCSWITCH_MAX_CHANGES];

    
};

#endif
