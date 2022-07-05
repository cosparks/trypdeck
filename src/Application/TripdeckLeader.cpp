#include <iostream>
#include <algorithm>

#include "TripdeckLeader.h"
#include "td_util.h"
#include "Clock.h"
#include "MockButton.h"

TripdeckLeader::TripdeckLeader(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckLeader::~TripdeckLeader() { }

void TripdeckLeader::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	// hook up button inputs with input manager (input manager will clean up heap objects)
	td_util::Command* digitalInputDelegate = new Delegate<TripdeckLeader, InputArgs>(this, &TripdeckLeader::_handleDigitalInput);
	#if RUN_MOCK_BUTTONS
	// TODO: remove test code
	_inputManager->addInput(new MockButton(LEADER_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	_inputManager->addInput(new MockButton(FOLLOWER_1_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	_inputManager->addInput(new MockButton(FOLLOWER_2_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	#else
	_inputManager->addInput(new InputDigitalButton(LEADER_BUTTON_ID, LEADER_BUTTON_PIN), digitalInputDelegate);
	_inputManager->addInput(new InputDigitalButton(FOLLOWER_1_BUTTON_ID, FOLLOWER_1_BUTTON_PIN), digitalInputDelegate);
	_inputManager->addInput(new InputDigitalButton(FOLLOWER_2_BUTTON_ID, FOLLOWER_2_BUTTON_PIN), digitalInputDelegate);
	#endif

	// TODO: add buttons which will perform full reset and shutdown
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
	args.state = _status.state;

	switch (_status.state) {
		case Connecting:
			// same as Connected
		case Connected:
			args.mediaOption = Both;
			args.playbackOption = MediaPlayer::Loop;
			break;
		case Wait:
			args.videoId = _mediaManager->getRandomVideoId(_status.state);
			args.ledId = _mediaManager->getRandomLedId(_status.state);
			args.mediaOption = None;
			args.playbackOption = MediaPlayer::Cycle;
			_updateStateFollowers(args);
			_setMediaUpdateUniversalAction(Both, MediaPlayer::Play);
			break;
		case Pulled:
			// pre-reveal stuff can be handled here
			#if STOP_VIDEO_ON_PULLED
			_mediaManager->stop(Video);
			args.mediaOption = None;
			#else
			args.mediaOption = Video;
			#endif

			args.playbackOption = MediaPlayer::OneShot;
			break; 
		case Reveal:
			if (_chainPulled)
				args.mediaOption = Both;
			else
				args.mediaOption = Led;
			args.ledId = _status.ledMedia;
			args.playbackOption = MediaPlayer::OneShot;
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

void TripdeckLeader::_handleSerialInput(const InputArgs& args) {
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
		case MEDIA_PLAYBACK_COMPLETE_HEADER:
			_handleMediaPlaybackCompleteMessage(id, args.buffer);
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
	stateArgs.state = Connected;
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

void TripdeckLeader::_handleMediaPlaybackCompleteMessage(char id, const std::string& buffer) {
	// Only handle cycling Led at the moment
	if (_parsePlaybackOption(buffer) == MediaPlayer::Cycle && _parseMediaOption(buffer) == Led) {
		TripdeckStateChangedArgs args = { };
		_populateStateArgsFromBuffer(buffer, args);
		_handleMediaPlayerPlaybackComplete(args);
	}
}

void TripdeckLeader::_updateStateFollower(char id, TripdeckStateChangedArgs& args) {
	std::string message = _populateBufferFromStateArgs(args, STATE_CHANGED_HEADER, id);
	_serial->transmit(message);
}

void TripdeckLeader::_updateStateFollowers(TripdeckStateChangedArgs& args) {
	for (auto const& pair : _nodeIdToStatus) {
		_updateStateFollower(pair.first, args);
	}
}

void TripdeckLeader::_updateMediaStateFollower(char id, TripdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
	std::string message = DEFAULT_MESSAGE;
	
	switch (state) {
		case MediaPlayer::Play:
			message[0] = PLAY_MEDIA_HEADER;
			break;
		case MediaPlayer::Stop:
			message[0] = STOP_MEDIA_HEADER;
			break;
		default:
			message[0] = PAUSE_MEDIA_HEADER;
			break;
	}

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
		message[ID_INDEX] = pair.first;
		_serial->transmit(message);
	}

	(_mediaManager->*localAction)(option);
}

void TripdeckLeader::_triggerLedAnimationForState(TripdeckState state, MediaPlayer::MediaPlaybackOption playbackOption) {
	TripdeckStateChangedArgs args = { };
	args.state = state;
	args.ledId = _mediaManager->getRandomLedId(state);
	args.mediaOption = Led;
	args.playbackOption = playbackOption;
	std::string message = _populateBufferFromStateArgs(args, PLAY_MEDIA_FROM_ARGS_HEADER);

	_updateMediaStateUniversal(Led, MediaPlayer::Stop);

	for (const auto& pair : _nodeIdToStatus) {
		// update follower media states
		message[ID_INDEX] = pair.first;
		_serial->transmit(message);
	}

	// update media state for this
	_mediaManager->updateStateLed(args);
}

void TripdeckLeader::_handleDigitalInput(const InputArgs& args) {
	if (args.buffer[0] != '1')
		return;
	
	// handle reset
	if (args.id == RESET_BUTTON_ID) {
		_handleReset();
		return;
	}

	// handle reset
	if (args.id == SHUTDOWN_BUTTON_ID) {
		_handleShutdown();
		return;
	}

	// back out if we are not in a state to receive chain pull
	if (_status.state == Wait || _status.state == Pulled)
		_handleChainPull(args.id);
}

void TripdeckLeader::_handleChainPull(char id) {
	bool newPull = false;

	if (id == LEADER_ID) {
		// TODO: Write internal set state method..
		// check if leader's chain has already been pulled
		if (_status.state == Pulled)
			return;
		
		newPull = true;

		// first chain pull -- handle internally
		_status.state = Pulled;
		_chainPulled = true;
		_onStateChanged();
	} else {
		// check if follower node's chain has already been pulled
		if (_nodeIdToStatus[id].state == Pulled)
			return;

		newPull = true;

		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "State changed for node with id: " << id << ".  Updating node" << std::endl;
		#endif

		// first chain pull -- update follower status
		_nodeIdToStatus[id].state = Pulled;

		// send stop Video message to Follower
		#if STOP_VIDEO_ON_PULLED
		_updateMediaStateFollower(id, Video, MediaPlayer::Stop);
		TripdeckMediaOption option = None;
		#else
		TripdeckMediaOption option = Video;
		#endif

		// send Pulled state update message to Followers (change led effect but not video)
		// at this point, we rely on Leader's internal representation of follower state to determine behavior
		TripdeckStateChangedArgs args = { };
		args.state = Pulled;
		args.mediaOption = option;
		args.playbackOption = MediaPlayer::OneShot;
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

	// check if all chains have been pulled
	// skip straight to pre-reveal if true
	if (_verifyAllPulled()) {
		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "Removing _executePreReveal to one shot actions" << std::endl;
		#endif

		_cancelOneShotActions();
		_executePreReveal();
		return;
	}

	if (newPull) {
		#if LED_ANIMATION_EACH_PULL
		_triggerLedAnimationForState(Pulled);
		#endif
	}
}

void TripdeckLeader::_handleReset() {
	// TODO
	// careful...
}

void TripdeckLeader::_handleShutdown() {
	// TODO
	// careful...
}

void TripdeckLeader::_handleMediaPlayerPlaybackComplete(const TripdeckStateChangedArgs& args) {
	// currently only called when Led && Cycle
	if (_status.state == Wait && args.state == Wait) {
		int64_t currentTime = Clock::instance().millis();
		if (currentTime > _lastPlaybackCompleteMessageMillis + PLAYBACK_MESSAGE_WAIT_INTERVAL) {
			_lastPlaybackCompleteMessageMillis = currentTime;

			// try twice to get random led animation:
			TripdeckStateChangedArgs newArgs = args;
			newArgs.ledId = _mediaManager->getRandomLedId(_status.state);

			if (newArgs.ledId == args.ledId)
				newArgs.ledId = _mediaManager->getRandomLedId(_status.state);
			
			std::string message = _populateBufferFromStateArgs(newArgs, PLAY_MEDIA_FROM_ARGS_HEADER);

			for (const auto& pair : _nodeIdToStatus) {
				message[ID_INDEX] = pair.first;
				_serial->transmit(message);
			}

			_mediaManager->updateStateLed(args);
		}
	}
}

void TripdeckLeader::_executePreReveal() {
	#if ENABLE_SERIAL_DEBUG
	// TODO: Remove debug code
	std::cout << "Executing pre reveal!" << std::endl;
	#endif

	#if PRE_REVEAL_TO_REVEAL_TIME
	// if we are using pre-reveal state, stop all media and add actual reveal to one shot actions
	_updateMediaStateUniversal(Both, MediaPlayer::Stop);
	_addOneShotAction(&TripdeckLeader::_executeReveal, PRE_REVEAL_TO_REVEAL_TIME);
	#else
	//trigger reveal right away
	_updateMediaStateUniversal(Video, MediaPlayer::Stop);
	_executeReveal();
	#endif

	// set next action, reveal
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
		args.state = Reveal;
		args.ledId = ledId;

		if (pair.second.state == Pulled)
			args.mediaOption = Both;
		else
			args.mediaOption = Led;

		_updateStateFollower(pair.first, args);
	}

	_status.ledMedia = ledId;
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
