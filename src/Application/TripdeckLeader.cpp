#include <iostream>
#include <algorithm>

#include "TripdeckLeader.h"
#include "Clock.h"

TripdeckLeader::TripdeckLeader(InputManager* inputManager, Serial* serial) : Tripdeck(inputManager, serial) { }

TripdeckLeader::~TripdeckLeader() { }

void TripdeckLeader::init() {
	Tripdeck::init();
	// hook up button inputs with callback
}

void TripdeckLeader::run() {
	switch (_currentState) {
		case Connecting:
			Tripdeck::run();
			_runStartup();
			break;
		case Connected:
			Tripdeck::run();
			break;
		case Wait:
			Tripdeck::run();
			break;
		case Pulled:
			Tripdeck::run();
			_notifyPulled();
			break;
		case Reveal:
			Tripdeck::run();
			_notifyReveal();
			break;
		default:
			// do nothing
			break;
	}
}

void TripdeckLeader::handleMediaChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckLeader::_onStateChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckLeader::_runStartup() {
	if (_nodeIds.size() < NUM_FOLLOWERS)
		return;

	if (Clock::instance().millis() > STARTUP_TIME) {
		_currentState = Wait;
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