# Trypdeck Basscoast

Software for an interactive multimedia fortune-telling installation.  This project is designed to compile and run on a raspberry pi.

### Building and Running the Project
#### Clone:
• navigate to the desired folder and in the command line of your raspberry pi enter:
```
$ git clone https://github.com/cosparks/tripdeck_basscoast.git
$ git pull
```
#### Install PIGPIO Library:
• to install PIGPIO, navigate to a folder where you store libraries/external resources and enter (you may have to include sudo before each command if you don't have permissions)
```
$ wget https://github.com/joan2937/pigpio/archive/master.zip
$ unzip master.zip
$ cd pigpio-master
$ make
$ sudo make install
```
NOTE: run these commands if the python part of the install fails:
```
$ sudo apt install python-setuptools python3-setuptools
$ sudo make install
```
#### (Optional) Test PIGPIO:
• check that pigpio is working correctly
```
sudo ./x_pigpio # check C I/F

sudo pigpiod    # start daemon

./x_pigpiod_if2 # check C      I/F to daemon
./x_pigpio.py   # check Python I/F to daemon
./x_pigs        # check pigs   I/F to daemon
./x_pipe        # check pipe   I/F to daemon
```
#### Install OpenCv:
```
...
```



### Dependencies:
openCV, pigpio, stl c++17
