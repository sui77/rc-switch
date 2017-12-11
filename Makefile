# make rc-switch library for Linux..

all: RCSwitch.so rctest rts

clean:
	rm -f RCSwitch.so rctest rts

RCSwitch.so: RCSwitch.cpp RCSwitch.h
	g++ -o $@ -shared -fPIC -DRCSwitchDisableReceiving $<

rctest: rctest.cpp RCSwitch.so
	g++ -o $@ -fPIC -DRCSwitchDisableReceiving rctest.cpp RCSwitch.so

rts: rts.c
	gcc -o $@ $<
