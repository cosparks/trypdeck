#include <iostream>
#include <algorithm>

#include "TripdeckBehaviorLeader.h"
#include "Clock.h"

TripdeckBehaviorLeader::TripdeckBehaviorLeader(InputManager* inputManager, Serial* serial) : TripdeckBehavior(inputManager, serial) { }

TripdeckBehaviorLeader::~TripdeckBehaviorLeader() { }

void TripdeckBehaviorLeader::init() {
	TripdeckBehavior::init();
	// hook up button inputs with callback
}

void TripdeckBehaviorLeader::run() {
	switch (_currentState) {
		case Connecting:
			TripdeckBehavior::run();
			_runStartup();
			break;
		case Connected:
			TripdeckBehavior::run();
			break;
		case Wait:
			TripdeckBehavior::run();
			break;
		case Pulled:
			TripdeckBehavior::run();
			_notifyPulled();
			break;
		case Reveal:
			TripdeckBehavior::run();
			_notifyReveal();
			break;
		default:
			// do nothing
			break;
	}
}

void TripdeckBehaviorLeader::handleMediaChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckBehaviorLeader::_onStateChanged(TripdeckStateChangedArgs& args) {

}

void TripdeckBehaviorLeader::_runStartup() {
	if (_nodeIds.size() < NUM_FOLLOWERS)
		return;

	if (Clock::instance().millis() > STARTUP_TIME) {
		_currentState = Wait;
	}
}

void TripdeckBehaviorLeader::_notifyPulled() {

}

void TripdeckBehaviorLeader::_notifyReveal() {

}

void TripdeckBehaviorLeader::_updateFollowers() {

}

void TripdeckBehaviorLeader::_handleSerialInput(InputArgs& args) {
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

void TripdeckBehaviorLeader::_notifyConnected(const std::string )

void TripdeckBehaviorLeader::_handleUserInput(InputArgs* data) {

}