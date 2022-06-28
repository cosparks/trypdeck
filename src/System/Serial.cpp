#include <iostream> // TODO: Remove debug code

#include <stdio.h>
#include <string.h>
#include <stdexcept>

#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#include "Serial.h"

#ifndef SERIAL_BAUD
#define SERIAL_BAUD B9600
#endif

Serial::Serial(std::string portName, int flag) : _portName(portName), _flag(flag){ }

Serial::~Serial() { }

void Serial::init() {
	_portNum = open(_portName.c_str(), _flag);
	struct termios tty;
	
	if (tcgetattr(_portNum, &tty) != 0) {
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
	}

	// Set all flags
	tty.c_cflag &= ~PARENB;			// Clear parity bit
	tty.c_cflag &= ~CSTOPB;			// Clear stop field -- Makes it so there is only one stop bit
	tty.c_cflag &= ~CSIZE;			// Clear size bits
	tty.c_cflag |= CS8; 				// 8 bits per byte
	tty.c_cflag &= ~CRTSCTS;			// Disable RTS/CTS hardware flow control
	tty.c_cflag |= CREAD | CLOCAL;	// Turn on READ & ignore ctrl lines

	// tty.c_lflag &= ~ICANON;			// disable canonical mode (\n not needed)
	tty.c_lflag &= ~ECHO;			// Disable echo
	tty.c_lflag &= ~ECHOE;			// Disable erasure
	tty.c_lflag &= ~ECHONL;			// Disable new-line echo
	tty.c_lflag &= ~ISIG;			// Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);								// Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); 	// Disable any special handling of received bytes

	// tty.c_oflag &= ~OPOST;		// Prevent special interpretation of output bytes (e.g. newline chars)
	// tty.c_oflag &= ~ONLCR;		// Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VTIME] = 50;				// Wait for up to 5s (50 deciseconds), returning as soon as any data is received.
	tty.c_cc[VMIN] = 1;					// return with minimum of one byte read

	// Set baud rate
	cfsetspeed(&tty, SERIAL_BAUD);

	// Save tty settings
	if (tcsetattr(_portNum, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}

void Serial::transmit(const std::string& data) {
	if (data.length() >= SERIAL_BUFFER_SIZE)
		throw std::runtime_error("Error: Data string is greater than maximum UART message size (including newline character)");

	memset(_buf, '\0', SERIAL_BUFFER_SIZE);
	strcpy(_buf, data.c_str());
	
	int32_t length = data.length();

	if (_buf[length] != '\n')
		_buf[length++] = '\n';

	int32_t ret = write(_portNum, _buf, length);

	if (ret < 0)
		throw std::runtime_error("unistd write error! Unable to write to serial port: " + std::string(strerror(errno)));

	std::cout << "Write complete -- wrote " << ret << " / " << data.length() + 1 <<  " Bytes requested from buffer: " << _buf << std::endl;
}

std::string Serial::receive() {
	char buf[SERIAL_BUFFER_SIZE] = { };
	int32_t ret = read(_portNum, buf, SERIAL_BUFFER_SIZE);
	if (ret < 0) {
		throw std::runtime_error("unistd read error! Unable to read from serial port: " + std::string(strerror(errno)));
	}

	// TODO: Remove debug code
	std::cout << "Read complete -- read " << ret << " bytes from buffer: " << buf << std::endl;

	return std::string(buf);
}