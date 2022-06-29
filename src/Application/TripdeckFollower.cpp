#include <iostream>

#include "TripdeckFollower.h"
#include "Clock.h"

TripdeckFollower::TripdeckFollower(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckFollower::~TripdeckFollower() { }

void TripdeckFollower::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	TripdeckStateChangedArgs args = { };
	args.newState = _currentState;
	_onStateChanged(args);
}

void TripdeckFollower::run() {
	while (_run) {
		switch (_currentState) {
			case Connecting:
				_runTimedAction(&TripdeckFollower::_sendConnectingMessage);
				break;
			case Connected:
				_runTimedAction(&TripdeckFollower::_sendStatusUpdate);
				break;
			case Wait:
				break;
			case Pulled:
				break;
			case Reveal:
				break;
			default:
				// do nothing
				break;
		}

		Tripdeck::run();
	}
}

void TripdeckFollower::_onStateChanged(TripdeckStateChangedArgs& args) {
	_mediaManager->updateState(args);
}

void TripdeckFollower::_sendConnectingMessage() {
	std::string data = STARTUP_NOTIFICATION_HEADER;
	data += ID;
	_serial->transmit(data);
}

void TripdeckFollower::_sendStatusUpdate() {
	std::string message = STATUS_UPDATE_HEADER;
	message += ID;
	message.append(std::to_string(_currentState));

	if (_status.videoMedia != 0)
		message.append("/" + _hashToHexString(_status.videoMedia));
	if (_status.ledMedia != 0)
		message.append("/" + _hashToHexString(_status.ledMedia));
		
	_serial->transmit(message);
}

void TripdeckFollower::_runTimedAction(void (TripdeckFollower::*action)(void), int64_t interval) {
	int64_t currentTime = Clock::instance().millis();
	if (currentTime >= _nextActionMillis) {
		_nextActionMillis = currentTime + interval;

		(this->*action)();
	}
}

void TripdeckFollower::_handleSerialInput(InputArgs& args) {
	if (args.buffer.length() < HEADER_LENGTH + 2) {
		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "Warning: Invalid message received -- length: " << args.buffer.length() << std::endl;
		#endif
		
		return;
	}
	
	// check if message is intended for this follower
	if (_parseId(args.buffer).compare(ID) == 0) {
		const std::string header = _parseHeader(args.buffer);

		if (header.compare(STATE_CHANGED_HEADER) == 0) {
			TripdeckStateChangedArgs stateArgs = { };

			if (_parseStateChangedMessage(args.buffer, stateArgs)) {
				_currentState = stateArgs.newState;
				_onStateChanged(stateArgs);
			}
		} else if (header.compare(PLAY_MEDIA_HEADER) == 0) {
			TripdeckMediaOption option = _parseMediaOption(args.buffer);
			_mediaManager->play(option);
		} else if (header.compare(STOP_MEDIA_HEADER)) {
			TripdeckMediaOption option = _parseMediaOption(args.buffer);
			_mediaManager->stop(option);
		} else if (header.compare(PAUSE_MEDIA_HEADER)) {
			TripdeckMediaOption option = _parseMediaOption(args.buffer);
			_mediaManager->pause(option);
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
	if (args.newState != _currentState) {
		// check for extra data in serial message
		if (_containsMediaHashes(buffer)) {
			MediaHashes hashes = _parseMediaHashes(buffer);

			if (hashes.videoHash != 0) {
				args.videoId = hashes.videoHash;
				args.syncVideo = true;
			}

			if (hashes.ledHash != 0) {
				args.ledId = hashes.ledHash;
				args.syncLeds = true;
			}
		}
		return true;
	}
	return false;
}