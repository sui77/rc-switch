#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <termios.h>
#include <sys/ioctl.h>

void doonoff(int fd, int onoff) {
	unsigned long req = onoff?TIOCMBIS:TIOCMBIC;
	int rts_flag = TIOCM_RTS;
	ioctl(fd, req, &rts_flag);
}

void docycle(int fd, unsigned long period) {
	unsigned long req = TIOCMBIS;
	int rts_flag = TIOCM_RTS;
	struct timespec tv = { 0, period };
	while (!nanosleep(&tv, NULL)) {
		ioctl(fd, req, &rts_flag);
		req = TIOCMBIS==req?TIOCMBIC:TIOCMBIS;
	}
	perror("nanosleep");
}

int main(int argc, char **argv) {
	char *serial="/dev/ttyS0";
	int arg, cycle=0, onoff=0;
	for (arg=1; arg<argc; arg++) {
		if (strncmp(argv[arg],"-d",2)==0)
			serial = argv[++arg];
		else if (strncmp(argv[arg],"cy",2)==0)
			cycle = 1;
		else if (strncmp(argv[arg],"on",2)==0)
			onoff = 1;
	}
	printf("serial port: %s, cycle=%d onoff=%d\n", serial, cycle, onoff);
	arg = open(serial, O_RDWR | O_NOCTTY);
	if (arg<0) {
		perror("opening serial port");
		return 1;
	}
	if (cycle)
		docycle(arg, 350000);
	else
		doonoff(arg, onoff);
	close(arg);
	return 0;
}
