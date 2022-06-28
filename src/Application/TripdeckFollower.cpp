#include "TripdeckFollower.h"
#include "Clock.h"

#define STARTUP_NOTIFICATION_INTERVAL 5000

TripdeckFollower::TripdeckFollower(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckFollower::~TripdeckFollower() { }

void TripdeckFollower::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();
}

void TripdeckFollower::run() {
	while (_run) {
		Tripdeck::run();

		switch (_currentState) {
			case Connecting:
				_notifyLeader();
				break;
			case Connected:
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
	}
}

void TripdeckFollower::handleMediaChanged(TripdeckStateChangedArgs& args) {
	// do nothing ??
}

void TripdeckFollower::_onStateChanged(TripdeckStateChangedArgs& args) {
	// _mediaManager->updateState(args);
}

void TripdeckFollower::_notifyLeader() {
	int64_t currentTime = Clock::instance().millis();
	if (currentTime >= _nextActionMillis) {
		_nextActionMillis = currentTime + STARTUP_NOTIFICATION_INTERVAL;
		std::string data = STARTUP_NOTIFICATION_HEADER;
		data += ID;
		_serial->transmit(data);
	}
}

void TripdeckFollower::_handleSerialInput(InputArgs& args) {
	// TODO: Remove debug code
	if (args.buffer.length() < HEADER_LENGTH) {
		throw std::runtime_error("Error: Invalid message received");
		return;
	}
	
	// check if message is intended for this follower
	if (args.buffer.substr(HEADER_LENGTH, 1).compare(ID) == 0) {
		const std::string header = args.buffer.substr(0, HEADER_LENGTH);

		if (header.compare(STATE_CHANGED_HEADER) == 0) {
			TripdeckStateChangedArgs stateChangedArgs = { };

			if (_parseStateChangedMessage(args.buffer, stateChangedArgs)) {
				_currentState = stateChangedArgs.newState;
				_onStateChanged(stateChangedArgs);
			}
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

	args.newState = (TripdeckState)std::stoi(buffer.substr(HEADER_LENGTH + 2, 1));

	// parse message data only if we are entering a new state
	if (args.newState != _currentState) {
		// check for extra data in serial message
		if (buffer.substr(HEADER_LENGTH + 3, 1).compare("/") == 0) {
			std::string mediaHashes = buffer.substr(HEADER_LENGTH + 4);
			int32_t slashIndex = mediaHashes.find("/");
			uint32_t videoHash = std::stoul(mediaHashes.substr(0, slashIndex), NULL, 16);
			uint32_t ledHash = std::stoul(mediaHashes.substr(slashIndex + 1));

			if (videoHash != 0) {
				args.videoId = videoHash;
				args.syncVideo = true;
			}

			if (ledHash != 0) {
				args.ledId = ledHash;
				args.syncLeds = true;
			}
		}
		return true;
	}
	return false;
}