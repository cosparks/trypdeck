#include <iostream>
#include <pigpio.h>

#include "Clock.h"

using namespace std;

#define PRINT_INTERVAL 1000

bool initializeGpio() {
	if (gpioInitialise() < 0) {
		cout << "PI GPIO Initialization failed" << endl;
		return false;
	}
	else {
		cout << "PI GPIO Initialization successful" << endl;
		return true;
	}
}

int main(int argv, char** argc) {
	if (!initializeGpio()) {
		return -1;
	}
	
	bool run = true;
	int64_t lastTime = Clock::instance().millis();
	while (run) {
		if (Clock::instance().millis() >= lastTime + PRINT_INTERVAL) {
			cout << "The current time in milliseconds is: " << Clock::instance().millis() << endl;
			cout << "The current time in seconds is: " << Clock::instance().seconds() << endl;
			cout << "Yayy" << endl;
			lastTime = Clock::instance().millis();
		}

	}
}
