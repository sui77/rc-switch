# rc-switch
Use your Arduino to operate remote radio controlled devices

##Download
https://github.com/sui77/rc-switch/releases/latest

##Wiki
https://github.com/sui77/rc-switch/wiki

##Info
###Send RC codes

Use your Arduino to operate remote radio controlled devices. This will most likely work with all popular low cost power outlet sockets.

All you need is a Arduino, a 315/433MHz AM transmitter  and one or more devices with a SC5262 / SC5272, HX2262 / HX2272, PT2262 / PT2272, EV1527, RT1527, FP1527 or HS1527 chipset. Also supports Intertechno outlets.

For Raspberry Pi support *(send only)* you need to install [wiringPi](https://projects.drogon.net/raspberry-pi/wiringpi/download-and-install/) and set it in your Makefile `CXXFLAGS = -DRC_SWITCH_RASPBERRY_PI` and `LDFLAGS  = -lwiringPi -lwiringPiDev -lm`

###Receive and decode RC codes

Find out what codes your remote is sending. Use your remote to control your Arduino.

All you need is a Arduino, a 315/433MHz AM receiver (altough there is no instruction yet, yes it is possible to hack an existing device) and a remote hand set.
