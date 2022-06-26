#include <iostream>
#include "TripdeckBehaviorLeader.h"


TripdeckBehaviorLeader::TripdeckBehaviorLeader(InputManager* inputManager, Serial* serial) : TripdeckBehavior(inputManager, serial) { }

TripdeckBehaviorLeader::~TripdeckBehaviorLeader() { }

void TripdeckBehaviorLeader::init() {
	TripdeckBehavior::init();
	_currentState = Startup;
	// hook up button inputs with callback
	_serialInput = new InputThreadedSerial(5, _serial);
	_serialInputDelegate = new SerialInputDelegate(this);
	_inputManager->addInput(_serialInput, _serialInputDelegate);
}

void TripdeckBehaviorLeader::run() {
	switch (_currentState) {
		case Startup:
			TripdeckBehavior::run();
			break;
		case Wait:
			_checkInputs();
			break;
		case Pulled:
			_notifyPulled();
			break;
		case Reveal:
			_notifyReveal();
			break;
		default:
			// do nothing
			break;
	}
}

void TripdeckBehaviorLeader::_onStateChanged(TripdeckStateChangedArgs& args) {
	if (_currentState == Startup) {
		_inputManager->removeInput(_serialInput);
		delete _serialInput;
		_serialInput = NULL;
	}
}

void TripdeckBehaviorLeader::_checkInputs() {

}

void TripdeckBehaviorLeader::_notifyPulled() {

}

void TripdeckBehaviorLeader::_notifyReveal() {

}

void TripdeckBehaviorLeader::_updateFollowers() {

}

void TripdeckBehaviorLeader::_handleSerialInput(const std::string& buffer) {
	std::cout << "Serial input received: " << buffer << std::endl;
	TripdeckStateChangedArgs args = { };
	_onStateChanged(args);
}

void TripdeckBehaviorLeader::_handleUserInput(InputData* data) {

}