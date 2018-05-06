#!/bin/bash
sudo git fetch origin
sudo git pull --rebase origin master
sudo CFLAGS="-Os -Wall -march=native -I/opt/AMDAPPSDK-3.0/include" LDFLAGS="-L/opt/amdgpu-pro/lib/x86_64-linux-gnu" ./configure --disable-git-version --disable-adl --prefix=/opt/sgminer-1.0.0
sudo make $(if $(THREADS="-j$(($(tail -c 2 /sys/devices/system/node/node0/cpulist 2>/dev/null)+2))"); then echo $THREADS; fi) 
