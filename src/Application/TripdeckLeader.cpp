#include <iostream>
#include <algorithm>

#include "TripdeckLeader.h"
#include "Clock.h"
#include "MockButton.h"

TripdeckLeader::TripdeckLeader(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckLeader::~TripdeckLeader() { }

void TripdeckLeader::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	// hook up button inputs with input manager
	// TODO: remove test code
	_inputManager->addInput(new MockButton(LEADER_BUTTON_ID, 5000, 10000), new TripdeckLeader::DigitalInputDelegate(this));
	_inputManager->addInput(new MockButton(FOLLOWER_1_BUTTON_ID, 5000, 10000), new TripdeckLeader::DigitalInputDelegate(this));
	// TODO: add button which will perform full reset
	
	_onStateChanged();
}

void TripdeckLeader::run() {
	while (_run) {
		Tripdeck::run();
		_runOneShotActions();

		switch (_status.state) {
			case Connecting:
				// same as Connected
			case Connected:
				_runTimedAction(this, &TripdeckLeader::_runStartup, 100);
				break;
			case Wait:
				break;
			case Pulled:
				_runTimedAction(this, &TripdeckLeader::_runPulled, 20);
				break;
			case Reveal:
				break;
			default:
				// Unknown state -- do nothing
				break;
		}
	}
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

bool TripdeckLeader::_verifySynced() {
	for (auto const& pair : _nodeIdToStatus) {
		if (!pair.second.connected)
			return false;
	}

	return true;
}

void TripdeckLeader::_runPulled() {
	if (!_executingPreReveal && _verifyAllPulled()) {
		// if all chains have been pulled, jump straight to pre-reveal

		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "Removing _executePreReveal to one shot actions" << std::endl;
		#endif

		_updateMediaStateUniversal(Both, MediaPlayer::Stop);
		_cancelOneShotActions();
		_executePreReveal();
	}
}

bool TripdeckLeader::_verifyAllPulled() {
	for (auto const& pair : _nodeIdToStatus) {
		if (pair.second.state != Pulled)
			return false;
	}

	return _status.state == Pulled;
}

void TripdeckLeader::_addOneShotAction(void (TripdeckLeader::*action)(void), int64_t wait) {
	_oneShotActions.push_back(std::make_pair(Clock::instance().millis() + wait, action));
}

void TripdeckLeader::_cancelOneShotActions() {
	_oneShotActions.clear();
}

void TripdeckLeader::_runOneShotActions() {
	if (_oneShotActions.size() > 0) {
		std::vector<int32_t> toDelete;
		uint8_t i = 0;
		int64_t currentTime = Clock::instance().millis();

		for (auto const& pair : _oneShotActions) {
			// if we are past this action's execution time, execute
			if (currentTime > pair.first) {
				(this->*pair.second)();
				toDelete.push_back(i);
			}
			i++;
		}

		for (int32_t index : toDelete) {
			_oneShotActions.erase(_oneShotActions.begin() + index);
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
			args.mediaOption = Both;
			args.loop = true;
			break;
		case Wait:
			args.videoId = _mediaManager->getRandomVideoId(_status.state);
			args.ledId = _mediaManager->getRandomLedId(_status.state);
			args.mediaOption = None;
			args.loop = true;
			_updateStateFollowers(args);
			_setMediaUpdateUniversalAction(Both, MediaPlayer::Play);
			break;
		case Pulled:
			// pre-reveal stuff can be handled here
			_mediaManager->stop(Video);
			args.mediaOption = None;
			args.loop = false;
			break;
		case Reveal:
			if (_chainPulled)
				args.mediaOption = Both;
			else
				args.mediaOption = Led;
			args.loop = false;
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

void TripdeckLeader::_handleSerialInput(InputArgs& args) {
	if (!_validateSerialMessage(args.buffer))
		return;

	char header = _parseHeader(args.buffer);
	char id = _parseId(args.buffer);

	switch (header) {
		case STARTUP_NOTIFICATION_HEADER:
			_receiveStartupNotification(id, args.buffer);
			break;
		case STATUS_UPDATE_HEADER:
			_receiveFollowerStatusUpdate(id, args.buffer);
			break;
		default:
			// do nothing
			break;
	}
}

void TripdeckLeader::_receiveStartupNotification(char id, const std::string& buffer) {
	// add node to map and send state update message
	if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end())
		_nodeIdToStatus[id] = TripdeckStatus { 0x0, 0x0, Both, Unknown, 0, false };

	// update follower to connected state and tell it to play new media upon receipt of message
	TripdeckStateChangedArgs stateArgs = { };
	stateArgs.newState = Connected;
	stateArgs.mediaOption = Both;
	_updateStateFollower(id, stateArgs);
}

void TripdeckLeader::_receiveFollowerStatusUpdate(char id, const std::string& buffer) {
	// update internal representation of node's state, including time
	int64_t currentTime = Clock::instance().millis();
	if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end()) {
		_nodeIdToStatus[id] = TripdeckStatus { 0x0, 0x0, _parseMediaOption(buffer), _parseState(buffer), currentTime, true };
	} else {
		_nodeIdToStatus[id].option = _parseMediaOption(buffer);
		_nodeIdToStatus[id].state = _parseState(buffer);
		_nodeIdToStatus[id].lastTransmitMillis = currentTime;
		_nodeIdToStatus[id].connected = true;
	}

	if (_containsMediaHashes(buffer)) {
		MediaHashes hashes = _parseMediaHashes(buffer);
		_nodeIdToStatus[id].videoMedia = hashes.videoHash;
		_nodeIdToStatus[id].ledMedia = hashes.ledHash;
	}
}

void TripdeckLeader::_updateStateFollower(char id, TripdeckStateChangedArgs& args) {
	std::string message = DEFAULT_MESSAGE;
	message[0] = STATE_CHANGED_HEADER;
	message[ID_INDEX] = id;
	message[STATE_INDEX] = _singleDigitIntToChar((int32_t)args.newState);
	message[MEDIA_OPTION_INDEX] = _singleDigitIntToChar((int32_t)args.mediaOption);
	message[LOOP_INDEX] = args.loop ? '1' : '0';
	
	if (args.videoId || args.ledId) {
		message.append("/" + _hashToHexString(_mediaManager->getRandomVideoId(args.newState)));
		message.append("/" + _hashToHexString(_mediaManager->getRandomLedId(args.newState)));
	}

	_serial->transmit(message);
}

void TripdeckLeader::_updateStateFollowers(TripdeckStateChangedArgs& args) {
	for (auto const& pair : _nodeIdToStatus) {
		_updateStateFollower(pair.first, args);
	}
}

void TripdeckLeader::_updateMediaStateFollower(char id, TripdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
	std::string message = DEFAULT_MESSAGE;
	message[0] = STOP_MEDIA_HEADER;
	message[ID_INDEX] = id;
	message[MEDIA_OPTION_INDEX] = _singleDigitIntToChar((int32_t)option);
	_serial->transmit(message);
}

void TripdeckLeader::_setMediaUpdateUniversalAction(TripdeckMediaOption option, MediaPlayer::MediaPlayerState state, int64_t wait) {
	_nextMediaActionOption = option;
	_nextMediaPlayerState = state;
	_addOneShotAction(&TripdeckLeader::_updateMediaStateUniversalAction, wait);
}

void TripdeckLeader::_updateMediaStateUniversalAction() {
	_updateMediaStateUniversal(_nextMediaActionOption, _nextMediaPlayerState);
}

void TripdeckLeader::_updateMediaStateUniversal(TripdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
	std::string message = DEFAULT_MESSAGE;
	void (TripdeckMediaManager::*localAction)(TripdeckMediaOption) = NULL;

	switch (state) {
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

	// set media option (follower will ignore state for these types of messages)
	message[MEDIA_OPTION_INDEX] = _singleDigitIntToChar((int32_t)option);

	for (const auto& pair : _nodeIdToStatus) {
		// copy node ID into string and transmit
		message[1] = pair.first;
		_serial->transmit(message);
	}

	(_mediaManager->*localAction)(option);
}

void TripdeckLeader::_handleDigitalInput(InputArgs& data) {
	// handle reset
	if (data.id == RESET_BUTTON_ID) {
		if (data.buffer[0] == '1')
			_handleReset();
		return;
	}

	// back out if we are not in a state to receive chain pull
	if (_status.state != Wait && _status.state != Pulled)
		return;
	
	// check if chain has been pulled
	if (data.buffer[0] == '1') {
		_handleChainPull(data.id);
	}
}

void TripdeckLeader::_handleChainPull(char id) {
	if (id == LEADER_ID) {
		// TODO: Write internal set state method..
		// check if leader's chain has already been pulled
		if (_status.state == Pulled)
			return;
		
		// first chain pull -- handle internally
		_status.state = Pulled;
		_onStateChanged();
	} else {
		// check if follower node's chain has already been pulled
		if (_nodeIdToStatus[id].state == Pulled)
			return;

		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "State changed for node with id: " << id << ".  Updating node" << std::endl;
		#endif

		// first chain pull -- update follower status
		_nodeIdToStatus[id].state = Pulled;

		// send stop Video message to Follower
		_updateMediaStateFollower(id, Video, MediaPlayer::Stop);

		// send Pulled state update message to Followers (change led effect but not video)
		// at this point, we rely on Leader's internal representation of follower state to determine behavior
		TripdeckStateChangedArgs args = { };
		args.newState = Pulled;
		args.mediaOption = None;
		args.loop = false;
		_updateStateFollower(id, args); 
	}

	// add one shot action for initialize pre-reveal
	if (!_revealTriggered) {
		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "Adding _executePreReveal to one shot actions" << std::endl;
		#endif
		
		_addOneShotAction(&TripdeckLeader::_executePreReveal, PULL_TO_PRE_REVEAL_TIME);
		_revealTriggered = true;
	}
}

void TripdeckLeader::_handleReset() {
	// TODO
	// careful...
}

void TripdeckLeader::_executePreReveal() {
	#if ENABLE_SERIAL_DEBUG
	// TODO: Remove debug code
	std::cout << "Executing pre reveal!" << std::endl;
	#endif

	_executingPreReveal = true;
	_updateMediaStateUniversal(Both, MediaPlayer::Stop);

	// TODO: Add some LED behaviors in here

	// set next action, reveal
	_addOneShotAction(&TripdeckLeader::_executeReveal, PRE_REVEAL_TO_REVEAL_TIME);
}

void TripdeckLeader::_executeReveal() {
	#if ENABLE_SERIAL_DEBUG
	// TODO: Remove debug code
	std::cout << "Executing reveal!" << std::endl;
	#endif

	// notify all nodes to play video and led animation
	uint32_t ledId = _mediaManager->getRandomLedId(Reveal);

	for (auto const& pair : _nodeIdToStatus) {
		TripdeckStateChangedArgs args = { };
		args.newState = Reveal;
		args.ledId = ledId;

		if (pair.second.state == Pulled)
			args.mediaOption = Both;
		else
			args.mediaOption = Led;

		_updateStateFollower(pair.first, args);
	}

	_executingPreReveal = false;
	_status.state = Reveal;
	_onStateChanged();

	_addOneShotAction(&TripdeckLeader::_returnToWait, REVEAL_TIME);
}

void TripdeckLeader::_returnToWait() {
	_revealTriggered = false;
	_chainPulled = false;

	_status.state = Wait;
	_onStateChanged();
}

// Digital Input Delegate
TripdeckLeader::DigitalInputDelegate::DigitalInputDelegate(TripdeckLeader* owner) : _owner(owner) { }

TripdeckLeader::DigitalInputDelegate::~DigitalInputDelegate() { }

void TripdeckLeader::DigitalInputDelegate::execute(CommandArgs args) {
	_owner->_handleDigitalInput(*((InputArgs*)args));
}
