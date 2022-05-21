#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <string>

class Serial {
	public:
		Serial(std::string portName, int flag, int bufferSize = 64);
		~Serial();
		void init();
		void transmit(std::string data);
		std::string receive();
	private:
		std::string _portName;
		int _flag;
		int _portNum = -1;
		int _bufferSize;
		char* _buf;
};

#endif