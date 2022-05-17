#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <string>

class Serial {
	public:
		Serial(char* port, int num, int bufferSize = 64);
		~Serial();
		void init();
		int transmit(std::string data);
		std::string receive();
	private:
		int _port = -1;
		int _bufferSize;
		char* _buf;
};

#endif