#!/bin/bash
sudo systemctl disable hciuart.service
sudo systemctl disable bluealsa.service
sudo systemctl disable bluetooth.service
sudo apt-get purge blues -y
sudo apt-get autoremove -y