#include "WProgram.h"

class RCSwitch {

  public:
    RCSwitch(int nPin);
    RCSwitch(int nPin, int nDelay);
    void switchOn(int nGroupNumber, int nSwitchNumber);
    void switchOff(int nGroupNumber, int nSwitchNumber);
    void switchOn(String sGroup, int nSwitchNumber);
    void switchOff(String sGroup, int nSwitchNumber);
    void send(String CodeWord);
  
  private:
    String getCodeWord(int nGroupNumber, int nSwitchNumber, boolean bStatus);
	String getCodeWord2(String sGroup, int nSwitchNumber, boolean bStatus);
    void send0();
    void send1();
    void sendF();
    void sendSync();

    int nPin;
    int nDelay;
    
    
};

