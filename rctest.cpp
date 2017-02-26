// Simple test program for RCSwitch.so, switch a Type B device on/off

#include <stdio.h>
#include "RCSwitch.h"

int main(int argc, char **argv) {
	bool on = false;
	const char *serial = "/dev/ttyS0";
	int grp = 1, chn = 1;
	for (int arg=1; arg<argc; arg++) {
		if (strncmp(argv[arg],"-d",2)==0)
			serial = argv[++arg];
		else if (strncmp(argv[arg],"-g",2)==0)
			grp = atoi(argv[++arg]);
		else if (strncmp(argv[arg],"-c",2)==0)
			chn = atoi(argv[++arg]);
		else if (strncmp(argv[arg],"on",2)==0)
			on = true;
		else if (strncmp(argv[arg],"off",3)==0)
			on = false;
		else
			return puts("usage: rctest [-d <serial device>] [-g <group>] [-c <channel>] [on|off]");
	}
	printf("Switching device %d:%d %s via %s\n", grp, chn, on?"on":"off", serial);
	RCSwitch *dev = new RCSwitch();
	dev->enableTransmit(serial);
	if (on)
		dev->switchOn(grp, chn);
	else
		dev->switchOff(grp, chn);
	return 0;
}

