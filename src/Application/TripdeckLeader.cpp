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
	_onStateChanged();
}

void TripdeckLeader::run() {
	while (_run) {
		Tripdeck::run();
		_runOneShotAction();

		switch (_status.state) {
			case Connecting:
				// same as Connected
			case Connected:
				_runTimedAction(this, &TripdeckLeader::_runStartup, 100);
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

void TripdeckLeader::_onStateChanged() {
	TripdeckStateChangedArgs args = { };
	args.newState = _status.state;

	switch (_status.state) {
		case Connecting:
			// same as Connected
		case Connected:
			args.loop = true;
			break;
		case Wait:
			args.videoId = _mediaManager->getRandomVideoId(_status.state);
			args.ledId = _mediaManager->getRandomLedId(_status.state);
			args.mediaOption = None;
			args.loop = true;
			_updateFollowers(args);
			_setMediaNotificationAction(Both, MediaPlayer::Play);
			break;
		case Pulled:
			_mediaManager->stop();
			break;
		case Reveal:
			break;
		default:
			// do nothing
			break;
	}

	#if ENABLE_SERIAL_DEBUG
	std::cout << "State changed: " << _status.state << std::endl;
	#endif

	_mediaManager->updateState(args);
}

void TripdeckLeader::_runStartup() {
	if (_nodeIdToStatus.size() < NUM_FOLLOWERS)
		return;
	
	if (!_followersSynced) {
		if (_verifySynced()) {
			_followersSynced = true;
			_status.state = Connected;
			_onStateChanged();
		}

		return;
	}

	if (Clock::instance().millis() > STARTUP_TIME) {
		_status.state = Wait;
		_onStateChanged();
	}
}

void TripdeckLeader::_updateFollowers(TripdeckStateChangedArgs& args) {
	for (auto const& pair : _nodeIdToStatus) {
		_updateFollowerState(pair.first, args);
	}
}

void TripdeckLeader::_handleSerialInput(InputArgs& args) {
	if (!_validateSerialMessage(args.buffer))
		return;

	char header = _parseHeader(args.buffer);
	const std::string id = _parseId(args.buffer);
	TripdeckStateChangedArgs stateArgs = { };

	switch (header) {
		case STARTUP_NOTIFICATION_HEADER:
			// add node to map and send state update message
			if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end())
				_nodeIdToStatus[id] = TripdeckStatus { 0x0, 0x0, None, Unknown, false };

			stateArgs.newState = Connected;
			_updateFollowerState(id, stateArgs);
			break;
		case STATUS_UPDATE_HEADER:
			// update internal representation of node's state
			if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end()) {
				_nodeIdToStatus[id] = TripdeckStatus { 0x0, 0x0, _parseMediaOption(args.buffer), _parseState(args.buffer), true };
			} else {
				_nodeIdToStatus[id].state = _parseState(args.buffer);
				_nodeIdToStatus[id].connected = true;
			}

			if (_containsMediaHashes(args.buffer)) {
				MediaHashes hashes = _parseMediaHashes(args.buffer);
				_nodeIdToStatus[id].videoMedia = hashes.videoHash;
				_nodeIdToStatus[id].ledMedia = hashes.ledHash;
			}
			break;
		default:
			// do nothing
			break;
	}
}

void TripdeckLeader::_updateFollowerState(const std::string& id, TripdeckStateChangedArgs& args) { 
	std::string message(HEADER_LENGTH, STATE_CHANGED_HEADER);
	message.append(id + "/" + std::to_string(args.newState) + "/" + std::to_string(args.mediaOption));
	
	if (args.videoId || args.ledId) {
		message.append("/" + _hashToHexString(_mediaManager->getRandomVideoId(args.newState)));
		message.append("/" + _hashToHexString(_mediaManager->getRandomLedId(args.newState)));
	}

	_serial->transmit(message);
}

void TripdeckLeader::_setOneShotAction(void (TripdeckLeader::*action)(void), int64_t wait) {
	_nextOneShotActionMillis = Clock::instance().millis() + wait;
	_oneShotAction = action;
}

void TripdeckLeader::_runOneShotAction() {
	if (_oneShotAction && Clock::instance().millis() > _nextOneShotActionMillis) {
		(this->*_oneShotAction)();
		_oneShotAction = NULL;
	}
}

void TripdeckLeader::_setMediaNotificationAction(TripdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
	_nextMediaActionOption = option;
	_nextMediaPlayerState = state;
	_setOneShotAction(&TripdeckLeader::_mediaNotificationAction, 5);
}

void TripdeckLeader::_mediaNotificationAction() {
	std::string message = DEFAULT_MESSAGE;
	void (TripdeckMediaManager::*localAction)(TripdeckMediaOption) = NULL;

	switch (_nextMediaPlayerState) {
		case MediaPlayer::Play:
			localAction = &TripdeckMediaManager::play;
			message[0] = PLAY_MEDIA_HEADER;
			break;
		case MediaPlayer::Stop:
			message[0] = STOP_MEDIA_HEADER;
			localAction = &TripdeckMediaManager::stop;
			break;
		default:
			message[0] = PAUSE_MEDIA_HEADER;
			localAction = &TripdeckMediaManager::pause;
			break;
	}

	message[MEDIA_OPTION_INDEX] = (char)('0' + (int32_t)_nextMediaActionOption);

	for (const auto& pair : _nodeIdToStatus) {
		// copy node ID into string and transmit
		message[1] = pair.first[0];
		_serial->transmit(message);
	}

	(_mediaManager->*localAction)(_nextMediaActionOption);
}

bool TripdeckLeader::_verifySynced() {
	for (auto const& pair : _nodeIdToStatus) {
		if (!pair.second.connected)
			return false;
	}

	return true;
}

void TripdeckLeader::_handleUserInput(InputArgs* data) {

}