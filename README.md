# rc-switch
Use your Arduino or Raspberry Pi to operate remote radio controlled devices

##Download
https://github.com/sui77/rc-switch/releases/latest

##Wiki
https://github.com/sui77/rc-switch/wiki

##Info
###Send RC codes

Use your Arduino or Raspberry Pi to operate remote radio controlled devices. This will most likely work with all popular low cost power outlet sockets. If yours doesn't work, you might need to adjust the pulse length for some to work.

All you need is a Arduino or Raspberry Pi, a 315/433MHz AM transmitter  and one or more devices with a SC5262 / SC5272, HX2262 / HX2272, PT2262 / PT2272, EV1527, RT1527, FP1527 or HS1527 chipset. Also supports Intertechno outlets.

###Receive and decode RC codes

Find out what codes your remote is sending. Use your remote to control your Arduino.

All you need is a Arduino, a 315/433MHz AM receiver (altough there is no instruction yet, yes it is possible to hack an existing device) and a remote hand set.

For Raspberry Pi, clone the https://github.com/ninjablocks/433Utils project to compile a sniffer tool and transmission commands.

