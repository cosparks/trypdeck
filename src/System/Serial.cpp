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
	_buf = new char[_bufferSize]();
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
	cfsetspeed(&tty, B19200);

	// Save tty settings
	if (tcsetattr(_portNum, TCSANOW, &tty) != 0) {
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
	}
}

void Serial::transmit(const std::string& data) {
	if (data.length() > (uint32_t)(_bufferSize - 1))
		throw std::runtime_error("Error: Data string is greater than maximum UART message size (including newline character)");

	strcpy(_buf, data.c_str());
	_buf[data.length()] = '\n';
	_buf[data.length() + 1] = '\0';

	if (write(_portNum, _buf, data.length() + 1) < 0)
		throw std::runtime_error(std::string("Error: Unable to write to serial port"));

	std::cout << "Write complete -- buffer: " << _buf << std::endl;
}

std::string Serial::receive() {
	char buf[_bufferSize] = { };
	int32_t ret = read(_portNum, buf, _bufferSize);
	if (ret < 0) {
		std::string error = _getSerialError(errno);
		throw std::runtime_error("unistd read " + error +  ": unable to read from serial port");
	}

	// TODO: Remove debug code
	std::cout << "Read complete -- read " << ret << " bytes from buffer: " << buf << std::endl;

	return std::string(buf);
}

const std::string Serial::_getSerialError(int32_t error) {
	const char* ret;
	switch (error) {
		case EAGAIN:
			ret = "EAGAIN | EWOULDBLOCK";
		case EBADF:
			ret = "EBADF";
		case EFAULT:
			ret = "EFAULT";
		case EINTR:
			ret = "EINTR";
		case EINVAL:
			ret = "EINVAL";
		case EIO:
			ret = "EIO";
		case EISDIR:
			ret = "EISDIR";
		default:
			return std::to_string(error);
	}
	return std::string(ret);
}