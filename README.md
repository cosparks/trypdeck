# Trypdeck Basscoast

Software for an interactive multimedia fortune-telling installation.  This project is designed to compile and run on a raspberry pi.

## Building and Running the Project
### Clone the Project:
• navigate to the desired folder and in the command line of your raspberry pi enter
```
$ git clone https://github.com/cosparks/trypdeck.git
$ git pull
```

### Run Scripts:
• to install project dependencies and disable bluetooth (which opens serial port ttyAM0), navigate to scripts folder and enter
```
$ sudo ./package-installer.sh
$ sudo ./disable-bluetooth.sh
```

### Enable GPIO
```
$ sudo raspi-config
```
• disable UART console and enable UART pins
• enable SPI

### Build and Run:
• navigate to `.../trypdeck/src` folder and enter
```
$ make trypdeck
$ sudo ./trypdeck
```

### Dependencies:
ffmpeg, vlc, pigpio, make
