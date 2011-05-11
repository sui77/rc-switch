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

#include "WProgram.h"

#define maxChanges 500

typedef void (*RCSwitchCallback)(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw);

class RCSwitch {

  public:
    RCSwitch(int nPin);
    RCSwitch(int nPin, int nDelay);
    void switchOn(int nGroupNumber, int nSwitchNumber);
    void switchOff(int nGroupNumber, int nSwitchNumber);
    void switchOn(String sGroup, int nSwitchNumber);
    void switchOff(String sGroup, int nSwitchNumber);

    void sendTriState(String Code);
    void send(unsigned long Code, unsigned int length);
    void send(char* Code);
    
    void enableReceive(int interrupt, RCSwitchCallback callback);
    void disableReceive();

  
  private:
    String getCodeWordB(int nGroupNumber, int nSwitchNumber, boolean bStatus);
    String getCodeWordA(String sGroup, int nSwitchNumber, boolean bStatus);
    void sendT0();
    void sendT1();
    void sendTF();
    void send0();
    void send1();
    void sendSync();

    
    static void receiveInterrupt();
    static RCSwitchCallback mCallback;
    int nInterrupt;
    int nPin;
    int nDelay;
    
};

