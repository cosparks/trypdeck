#include <iostream>

#include "TripdeckFollower.h"

TripdeckFollower::TripdeckFollower(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckFollower::~TripdeckFollower() { }

void TripdeckFollower::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	TripdeckStateChangedArgs args = { };
	args.newState = _status.state;
	args.mediaOption = Both;
	_onStateChanged(args);
}

void TripdeckFollower::run() {
	while (_run) {
		Tripdeck::run();

		switch (_status.state) {
			case Connecting:
				_runTimedAction(this, &TripdeckFollower::_sendConnectingMessage);
				break;
			case Connected:
				// behavior is same as Wait
			case Wait:
				_runTimedAction(this, &TripdeckFollower::_sendStatusUpdate);
				break;
			case Pulled:
				break;
			case Reveal:
				break;
			default:
				// do nothing
				break;
		}
	}
}

void TripdeckFollower::_onStateChanged(TripdeckStateChangedArgs& args) {
	_status.videoMedia = args.videoId;
	_status.ledMedia = args.ledId;

	#if ENABLE_SERIAL_DEBUG
	// TODO: Remove debug code
	std::cout << "State changed: " << _status.state << std::endl;
	#endif

	_mediaManager->updateState(args);
}

void TripdeckFollower::_sendConnectingMessage() {
	std::string data(HEADER_LENGTH, STARTUP_NOTIFICATION_HEADER);
	data += ID;
	_serial->transmit(data);
}

void TripdeckFollower::_sendStatusUpdate() {
	std::string message(HEADER_LENGTH, STATUS_UPDATE_HEADER);
	message += ID;
	message.append("/" + std::to_string(_status.state));

	_serial->transmit(message);
}

void TripdeckFollower::_handleSerialInput(InputArgs& args) {
	if (!_validateSerialMessage(args.buffer))
		return;
	
	// check if message is intended for this follower
	if (_parseId(args.buffer)== ID) {
		
		char header = _parseHeader(args.buffer);
		TripdeckStateChangedArgs stateArgs = { };
		TripdeckMediaOption mediaOption = { };

		switch (header) {
			case STATE_CHANGED_HEADER:
				if (_parseStateChangedMessage(args.buffer, stateArgs)) {
					_status.state = stateArgs.newState;
					_onStateChanged(stateArgs);
				}
				break;
			case PLAY_MEDIA_HEADER:
				mediaOption = _parseMediaOption(args.buffer);
				_mediaManager->play(mediaOption);
				break;
			case STOP_MEDIA_HEADER:
				mediaOption = _parseMediaOption(args.buffer);
				_mediaManager->stop(mediaOption);
				break;
			case PAUSE_MEDIA_HEADER:
				mediaOption = _parseMediaOption(args.buffer);
				_mediaManager->pause(mediaOption);
				break;
		}
	} else {
		// if transmission is not for us, pass it on
		_serial->transmit(args.buffer);
	}
}

// returns true and loads StateChangedArgs if entering new state, false otherwise
bool TripdeckFollower::_parseStateChangedMessage(const std::string& buffer, TripdeckStateChangedArgs& args) {
	if (buffer.length() < 6)
		throw std::runtime_error("Error: Invalid state changed message.  State message length must be >= 6");

	args.newState = _parseState(buffer);

	// parse message data only if we are entering a new state
	if (args.newState != _status.state) {
		args.mediaOption = _parseMediaOption(buffer);

		// check for extra data in serial message
		if (_containsMediaHashes(buffer)) {
			MediaHashes hashes = _parseMediaHashes(buffer);
			args.videoId = hashes.videoHash;
			args.ledId = hashes.ledHash;
		}

		return true;
	}
	return false;
}