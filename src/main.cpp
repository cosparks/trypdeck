#include <iostream>
#include <pigpio.h>

using namespace std;

int main(int argv, char** argc) {
	if (gpioInitialise() < 0) {
		cout << "PI GPIO Initialization failed" << endl;
		return -1;
	}
	else {
		cout << "PI GPIO Initialization successful" << endl;
	}
	
	
}
