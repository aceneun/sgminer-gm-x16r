#!/bin/bash
sudo git fetch origin
sudo git pull --rebase origin master
sudo CFLAGS="-O2 -Wall -march=native -std=gnu99" ./configure --disable-adl
sudo make $(if $(THREADS="-j$(($(tail -c 2 /sys/devices/system/node/node0/cpulist 2>/dev/null)+2))"); then echo $THREADS; fi)
