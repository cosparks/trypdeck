#!/bin/bash
sudo apt update && sudo apt upgrade -y
sudo apt-get -y install build-essential cmake libvlc-dev pulseaudio
sudo apt install pkg-config
mkdir ~/resources
cd ~/resources
sudo wget https://github.com/joan2937/pigpio/archive/master.zip
unzip master.zip
rm master.zip
cd pigpio-master
make
sudo apt install python-setuptools python3-setuptools
sudo make install
