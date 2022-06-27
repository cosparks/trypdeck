#include <iostream>
#include <algorithm>

#include "TripdeckLeader.h"
#include "Clock.h"

TripdeckLeader::TripdeckLeader(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckLeader::~TripdeckLeader() { }

void TripdeckLeader::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	// hook up button inputs with callback
}

void TripdeckLeader::run() {
	while (_run) {
		Tripdeck::run();

		switch (_currentState) {
			case Connecting:
				_runStartup();
				break;
			case Connected:
				break;
			case Wait:
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

}

void TripdeckLeader::handleMediaChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckLeader::_onStateChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckLeader::_runStartup() {
	if (_nodeIds.size() < NUM_FOLLOWERS)
		return;
	else

	if (Clock::instance().millis() > STARTUP_TIME && _currentState == Connected) {
		
	}
}

void TripdeckLeader::_notifyPulled() {

}

void TripdeckLeader::_notifyReveal() {

}

void TripdeckLeader::_updateFollowers() {

}

void TripdeckLeader::_handleSerialInput(InputArgs& args) {
	std::cout << "Serial input received from id " << args.id << ": " << args.buffer << std::endl;

	// check header
	if (args.buffer.substr(0, HEADER_LENGTH).compare(STARTUP_NOTIFICATION_HEADER) == 0) {
		std::string id = args.buffer.substr(HEADER_LENGTH, 1);
		bool containsId = false;

		for (const std::string& nodeId : _nodeIds) {
			if (nodeId.compare(id) == 0) {
				containsId = true;
				break;
			}
		}

		if (!containsId) {
			_nodeIds.push_back(id);
			TripdeckStateChangedArgs args = { };
			args.newState = Connected;
		}
	} else {
		// if transmission is not for us, pass it on
		_serial->transmit(args.buffer);
	}
}

void TripdeckLeader::_notifyConnected(const std::string& id) { 

}

void TripdeckLeader::_handleUserInput(InputArgs* data) {

}