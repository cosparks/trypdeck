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
	// TODO: Remove debug code
	std::cout << "Leader has received message: " << args.buffer << std::endl;
	if (args.buffer.length() < HEADER_LENGTH) {
		std::cout << "Invalid Message" << std::endl;
		return;
	}

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
			// TODO: Remove debug code
			std::cout << "Node added with id: " << id << std::endl;
			_nodeIds.push_back(id);
		}

		TripdeckStateChangedArgs args = { };
		args.newState = Connected;
		_updateNodeState(id, args);
	} else {
		// if transmission is not for us, pass it on
		// _serial->transmit(args.buffer);
	}
}

void TripdeckLeader::_updateNodeState(const std::string& id, TripdeckStateChangedArgs& args) { 
	std::string message(STATE_CHANGED_HEADER);
	message.append(id + "/" + std::to_string(args.newState));
	
	if (args.syncVideo)
		message.append("/" + std::to_string(_mediaManager->getRandomVideoId(args.newState)));
	if (args.syncLeds)
		message.append("/" + std::to_string(_mediaManager->getRandomLedId(args.newState)));

	// TODO: Remove debug code
	std::cout << "Startup message received!  Updating node state with UART message: " << message << std::endl;

	_serial->transmit(message);
}

void TripdeckLeader::_handleUserInput(InputArgs* data) {

}