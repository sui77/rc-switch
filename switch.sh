#! /bin/sh
# Poke the rctest application a couple of times to ensure state is reached
HERE=`dirname $0`
LD_LIBRARY_PATH=$HERE $HERE/rctest $*
sleep 3
LD_LIBRARY_PATH=$HERE $HERE/rctest $*
sleep 3
LD_LIBRARY_PATH=$HERE $HERE/rctest $*
$HERE/rts off
