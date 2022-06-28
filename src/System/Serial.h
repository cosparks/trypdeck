#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <string>
#include <fcntl.h> // Contains file controls like O_RDWR

#include "settings.h"

class Serial {
	public:
		Serial(std::string portName, int32_t flag, int32_t bufferSize = 64);
		~Serial();
		void init();
		void transmit(const std::string& data);
		std::string receive();
	private:
		std::string _portName;
		int32_t _flag;
		int32_t _portNum = -1;
		int32_t _bufferSize;
		char _buf[SERIAL_BUFFER_SIZE];
};

#endif