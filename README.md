# Trypdeck Basscoast

![image](https://user-images.githubusercontent.com/20360622/191133995-b9554fdb-050f-4551-8bd6-64d7be6bdde0.png)

Software for interactive sculpture.  This project is designed to compile and run on a raspberry pi zero w 2

## Building and Running the Project
### Clone the Project:
• navigate to the desired folder and in the command line of your raspberry pi enter
```
$ git clone https://github.com/cosparks/tripdeck_basscoast.git
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
• navigate to `.../tripdeck_basscoast/src` folder and enter
```
$ make tripdeck
$ sudo ./tripdeck
```

### Dependencies:
ffmpeg, vlc, pigpio, make
