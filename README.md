# rc-switch
[![Build Status](https://travis-ci.org/Attila-FIN/rc-switch.svg?branch=master)](https://travis-ci.org/Attila-FIN/rc-switch)


Use your Arduino or [Raspberry Pi](https://github.com/r10r/rcswitch-pi) to operate remote radio controlled devices

## Download
Original:
https://github.com/sui77/rc-switch/releases/latest

rc-switch is also listed in the arduino library manager.

Modified to support Nexa and Everflourish
https://github.com/perivar/rc-switch

Modified to support for Cixi Yidong Electronics protocol.
Remote control switches are sold under brands AXXEL, Telco, EVOLOGY, CONECTO, mumbi, Manax etc.
More info about the protocol:
http://ipfone.hu/reverse-engineering-the-433mhz-cixi-yidong-electronics-protocol/

## Wiki
https://github.com/sui77/rc-switch/wiki

## Info
### Send RC codes

Use your Arduino or Raspberry Pi to operate remote radio controlled devices.
This will most likely work with all popular low cost power outlet sockets. If
yours doesn't work, you might need to adjust the pulse length.

All you need is a Arduino or Raspberry Pi, a 315/433MHz AM transmitter and one
or more devices with one of the supported chipsets:

 - SC5262 / SC5272
 - HX2262 / HX2272
 - PT2262 / PT2272
 - EV1527 / RT1527 / FP1527 / HS1527 
 - Intertechno outlets
 - HT6P20X
 - Everflourish like EMW100T
 - Nexa like LMLT-711

### Receive and decode RC codes

Find out what codes your remote is sending. Use your remote to control your
Arduino.

All you need is an Arduino, a 315/433MHz AM receiver (altough there is no
instruction yet, yes it is possible to hack an existing device) and a remote
hand set.

For the Raspberry Pi, clone the https://github.com/ninjablocks/433Utils project to
compile a sniffer tool and transmission commands.
