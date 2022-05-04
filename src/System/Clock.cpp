#include "Clock.h"

Clock* Clock::_instance;

Clock::Clock() {
	_startTime = _currentTimeInMilliseconds();
}

Clock& Clock::instance() {
	if (_instance == NULL) {
		_instance = new Clock();
	}
	return *_instance;
}

Clock::~Clock() { }

void Clock::reset() {
	_startTime = _currentTimeInMilliseconds();
}

int64_t Clock::millis() {
	return _currentTimeInMilliseconds() - _startTime;
}

int64_t Clock::seconds() {
	return millis() / 1000;
}

int64_t Clock::_currentTimeInMilliseconds() {
	auto timePoint = chrono::system_clock::now();
	return chrono::duration_cast<chrono::milliseconds>(timePoint.time_since_epoch()).count();
}