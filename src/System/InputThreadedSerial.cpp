#include <thread>

#include "InputThreadedSerial.h"

InputThreadedSerial::InputThreadedSerial(uint32_t id, Serial* serial) : Input(id), _serial(serial) { }

InputThreadedSerial::~InputThreadedSerial() { }

bool InputThreadedSerial::read() {
	_stateMutex.lock();
	if (_currentlyReading) {
		_stateMutex.unlock();
		return false;
	}

	if (_dataAvailable) {
		_stateMutex.unlock();
		return true;
	}

	if (!_dataAvailable) {
		_currentlyReading = true;

		_stateMutex.unlock();
		std::thread reader(&InputThreadedSerial::_readInternal, this);
		reader.detach();
	}

	return false;
}

void InputThreadedSerial::_readInternal() {
	_data = _serial->receive();

	_stateMutex.lock();
	_currentlyReading = false;
	_dataAvailable = true;
	_stateMutex.unlock();
}

void InputThreadedSerial::_setCurrentlyReading(bool value) {
	_currentlyReading = value;
}

bool InputThreadedSerial::_getCurrentlyReading() {
	return _currentlyReading;
}

void InputThreadedSerial::_setDataAvailable(bool value) {
	_dataAvailable = value;
}

bool InputThreadedSerial::_getDataAvailable() {
	return _dataAvailable;
}
