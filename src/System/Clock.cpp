#include "Clock.h"

Clock* Clock::_instance;

Clock::Clock() {
	_startTimeMillis = _currentTimeInMilliseconds();
	_startTimeMicros = _currentTimeInMicroseconds();
}

Clock& Clock::instance() {
	if (_instance == NULL) {
		_instance = new Clock();
	}
	return *_instance;
}

Clock::~Clock() {
	delete _instance;
}

void Clock::reset() {
	_startTimeMillis = _currentTimeInMilliseconds();
	_startTimeMicros = _currentTimeInMicroseconds();
}

int64_t Clock::millis() {
	return _currentTimeInMilliseconds() - _startTimeMillis;
}

int64_t Clock::micros() {
	return _currentTimeInMicroseconds() - _startTimeMicros;
}

int64_t Clock::seconds() {
	return millis() / 1000;
}

int64_t Clock::epochMillis() {
	return _currentTimeInMilliseconds();
}

int64_t Clock::_currentTimeInMilliseconds() {
	auto timePoint = chrono::system_clock::now();
	return chrono::duration_cast<chrono::milliseconds>(timePoint.time_since_epoch()).count();
}

int64_t Clock::_currentTimeInMicroseconds() {
	auto timePoint = chrono::system_clock::now();
	return chrono::duration_cast<chrono::microseconds>(timePoint.time_since_epoch()).count();
}