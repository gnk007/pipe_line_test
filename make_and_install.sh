#!/bin/sh -l

mkdir install
cd bb-kvstore-master
autoreconf -i
../bb-kvstore-master/configure --prefix=~/install
make -j 4 install
