#! /bin/bash
# Poke the rctest application a couple of times to ensure state is reached
HERE=`dirname $0`
CNT=3
[ -n "$SWITCHES" ] && CNT=$SWITCHES
while [ $CNT -gt 0 ]; do
	LD_LIBRARY_PATH=$HERE $HERE/rctest $*
	sleep 3
	CNT=$(($CNT - 1))
done
$HERE/rts off
