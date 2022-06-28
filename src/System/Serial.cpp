#include <iostream> // TODO: Remove debug code

#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include "Serial.h"

Serial::Serial(std::string portName, int flag, int bufferSize) : _portName(portName), _flag(flag), _bufferSize(bufferSize) { }

Serial::~Serial() {
	delete[] _buf;
}

void Serial::init() {
	_portNum = open(_portName.c_str(), _flag);
	_buf = new char[_bufferSize];
	struct termios tty;
	
	if (tcgetattr(_portNum, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	// Set all flags
	tty.c_cflag &= ~PARENB;			// Clear parity bit
	tty.c_cflag &= ~CSTOPB;			// Clear stop field
	tty.c_cflag &= ~CSIZE;			// Clear size bits
	tty.c_cflag |= CS8; 			// 8 bits per byte
	tty.c_cflag &= ~CRTSCTS;		// Disable RTS/CTS hardware flow control
	tty.c_cflag |= CREAD | CLOCAL;	// Turn on READ & ignore ctrl lines

	// tty.c_lflag &= ~ICANON;			// disable canonical mode (\n not needed)
	tty.c_lflag &= ~ECHO;			// Disable echo
	tty.c_lflag &= ~ECHOE;			// Disable erasure
	tty.c_lflag &= ~ECHONL;			// Disable new-line echo
	tty.c_lflag &= ~ISIG;			// Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);								// Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); 	// Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST;		// Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR;		// Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VTIME] = 50;				// Wait for up to 5s (50 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = _bufferSize;		// Minimum serial message length is same as buffer size

	// Set baud rate
	cfsetspeed(&tty, B9600);

	// Save tty settings
	if (tcsetattr(_portNum, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}

void Serial::transmit(const std::string& data) {
	if (data.length() > (uint32_t)(_bufferSize - 1))
		throw std::runtime_error("Error: Data string is greater than maximum UART message size (including newline character)");

	char buf[_bufferSize] = { };
	strcpy(buf, data.c_str());
	buf[data.length()] = '\n';

	// TODO: Remove debug code
	std::cout << "Serial::transmitting message: " << buf << std::endl;

	if (write(_portNum, buf, _bufferSize) < 0)
		throw std::runtime_error(std::string("Error: Unable to write to serial port"));
}

std::string Serial::receive() {
	char buf[_bufferSize] = { };
	read(_portNum, buf, _bufferSize);

	// TODO: Remove debug code
	std::cout << "Serial::received message: " << buf << std::endl;

	return std::string(buf);
}