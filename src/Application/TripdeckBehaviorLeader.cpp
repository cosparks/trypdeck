#include <iostream>
#include "TripdeckBehaviorLeader.h"

TripdeckBehaviorLeader::TripdeckBehaviorLeader(InputManager* inputManager, Serial* serial) : TripdeckBehavior(inputManager, serial) { }

TripdeckBehaviorLeader::~TripdeckBehaviorLeader() { }

void TripdeckBehaviorLeader::init() {
	TripdeckBehavior::init();
	// hook up button inputs with callback
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

void TripdeckBehaviorLeader::_handleSerialInput(InputArgs& args) {
	std::cout << "Serial input received from id " << args.id << ": " << args.buffer << std::endl;
}

void TripdeckBehaviorLeader::_handleUserInput(InputArgs* data) {

}