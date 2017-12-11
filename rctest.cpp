// Simple test program for RCSwitch.so, switch a Type B device on/off

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "RCSwitch.h"

class IO : public DigitalIO {
public:
	IO(const char *serial) {
		fd = open(serial, O_RDWR | O_NOCTTY);
	}
	~IO() {
		if (fd > 0)
			close(fd);
	}

	virtual void delayMicroseconds(unsigned int usec) {
		struct timespec tv = { usec / 1000000, (usec % 1000000) * 1000 };
		nanosleep(&tv, NULL);
	}

	virtual void digitalWrite(uint8_t level) {
		unsigned long req = HIGH==level?TIOCMBIS:TIOCMBIC;
		int rts_flag = TIOCM_RTS;
		ioctl(fd, req, &rts_flag);
	}
private:
	int fd;
};

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
	IO *io = new IO(serial);
	dev->enableTransmit(io);
	if (on)
		dev->switchOn(grp, chn);
	else
		dev->switchOff(grp, chn);
	return 0;
}

