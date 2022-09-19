#include <iostream>
#include <algorithm>

#include "TrypdeckLeader.h"
#include "td_util.h"
#include "Clock.h"
#include "Index.h"
#include "MockButton.h"
#include "InputDigitalButton.h"

TrypdeckLeader::TrypdeckLeader(TrypdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Trypdeck(mediaManager, inputManager, serial) { }

TrypdeckLeader::~TrypdeckLeader() { }

void TrypdeckLeader::init() {
	// sets state to Connecting and _run to true
	Trypdeck::init();

	td_util::Command* digitalInputDelegate = new Delegate<TrypdeckLeader, InputArgs>(this, &TrypdeckLeader::_handleDigitalInput);

	// hook up button inputs with input manager (input manager will clean up heap objects)
	#if RUN_MOCK_BUTTONS
	// TODO: remove test code
	_inputManager->addInput(new MockButton(LEADER_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	_inputManager->addInput(new MockButton(FOLLOWER_1_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	_inputManager->addInput(new MockButton(FOLLOWER_2_BUTTON_ID, MOCK_BUTTON_RANDOM_MIN_MILLIS, MOCK_BUTTON_RANDOM_MAX_MILLIS), digitalInputDelegate);
	#elif not ENABLE_VISUAL_DEBUG and not RUN_MOCK_BUTTONS
	_inputManager->addInput(new InputDigitalButton(LEADER_BUTTON_ID, LEADER_BUTTON_PIN), digitalInputDelegate);
	_inputManager->addInput(new InputDigitalButton(FOLLOWER_1_BUTTON_ID, FOLLOWER_1_BUTTON_PIN), digitalInputDelegate);
	_inputManager->addInput(new InputDigitalButton(FOLLOWER_2_BUTTON_ID, FOLLOWER_2_BUTTON_PIN), digitalInputDelegate);
	_inputManager->addInput(new InputDigitalButton(SHUTDOWN_BUTTON_ID, SHUTDOWN_BUTTON_PIN), digitalInputDelegate);
	#endif

	// TODO: add buttons which will perform full reset and shutdown
	_onStateChanged();
}

void TrypdeckLeader::run() {
	while (_run) {
		Trypdeck::run();
		_runOneShotActions();

		switch (_status.state) {
			case Connecting:
				// same as Connected
			case Connected:
				_runTimedAction(this, &TrypdeckLeader::_runStartup, 100);
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

void TrypdeckLeader::_runStartup() {
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

bool TrypdeckLeader::_verifySynced() {
	for (auto const& pair : _nodeIdToStatus) {
		if (!pair.second.connected)
			return false;
	}

	return true;
}

bool TrypdeckLeader::_verifyAllPulled() {
	for (auto const& pair : _nodeIdToStatus) {
		if (pair.second.state != Pulled)
			return false;
	}

	return _status.state == Pulled;
}

void TrypdeckLeader::_addOneShotAction(void (TrypdeckLeader::*action)(void), int64_t wait) {
	_oneShotActions.push_back(std::make_pair(Clock::instance().millis() + wait, action));
}

void TrypdeckLeader::_cancelOneShotActions() {
	_oneShotActions.clear();
}

void TrypdeckLeader::_runOneShotActions() {
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

void TrypdeckLeader::_onStateChanged() {
	TrypdeckStateChangedArgs args = { };
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
			args.mediaOption = Video;
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

	#if ENABLE_MEDIA_DEBUG
	std::cout << "State changed: " << _status.state << std::endl;
	#endif

	_mediaManager->updateState(args);
}

void TrypdeckLeader::_handleSerialInput(const InputArgs& args) {
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

void TrypdeckLeader::_receiveStartupNotification(char id, const std::string& buffer) {
	// add node to map and send state update message
	if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end())
		_nodeIdToStatus[id] = TrypdeckStatus { 0x0, 0x0, Both, Unknown, 0, false };

	// update follower to connected state and tell it to play new media upon receipt of message
	TrypdeckStateChangedArgs stateArgs = { };
	stateArgs.state = Connected;
	stateArgs.mediaOption = Both;
	_updateStateFollower(id, stateArgs);
}

void TrypdeckLeader::_receiveFollowerStatusUpdate(char id, const std::string& buffer) {
	// update internal representation of node's state, including time
	int64_t currentTime = Clock::instance().millis();
	if (_nodeIdToStatus.find(id) == _nodeIdToStatus.end()) {
		_nodeIdToStatus[id] = TrypdeckStatus { 0x0, 0x0, _parseMediaOption(buffer), _parseState(buffer), currentTime, true };
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

void TrypdeckLeader::_handleMediaPlaybackCompleteMessage(char id, const std::string& buffer) {
	// Only handle cycling Led at the moment
	if (_parsePlaybackOption(buffer) == MediaPlayer::Cycle && _parseMediaOption(buffer) == Led) {
		TrypdeckStateChangedArgs args = { };
		_populateStateArgsFromBuffer(buffer, args);
		_handleMediaPlayerPlaybackComplete(args);
	}
}

void TrypdeckLeader::_updateStateFollower(char id, TrypdeckStateChangedArgs& args) {
	std::string message = _populateBufferFromStateArgs(args, STATE_CHANGED_HEADER, id);
	_serial->transmit(message);
}

void TrypdeckLeader::_updateStateFollowers(TrypdeckStateChangedArgs& args) {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Updating followers for state " << _status.state;
	if (args.videoId)
		std::cout << "\n\t-- video file:  " << Index::instance().getSystemPath(args.videoId);
	if (args.ledId)
		std::cout << "\n\t-- led file:  " << Index::instance().getSystemPath(args.ledId) << std::endl;
	#endif

	for (auto const& pair : _nodeIdToStatus) {
		_updateStateFollower(pair.first, args);
	}
}

void TrypdeckLeader::_updateMediaStateFollower(char id, TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
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

void TrypdeckLeader::_setMediaUpdateUniversalAction(TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state, int64_t wait) {
	_nextMediaActionOption = option;
	_nextMediaPlayerState = state;
	_addOneShotAction(&TrypdeckLeader::_updateMediaStateUniversalAction, wait);
}

void TrypdeckLeader::_updateMediaStateUniversalAction() {
	_updateMediaStateUniversal(_nextMediaActionOption, _nextMediaPlayerState);
}

void TrypdeckLeader::_updateMediaStateUniversal(TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state) {
	std::string message = DEFAULT_MESSAGE;
	void (TrypdeckMediaManager::*localAction)(TrypdeckMediaOption) = NULL;

	switch (state) {
		case MediaPlayer::Play:
			localAction = &TrypdeckMediaManager::play;
			message[0] = PLAY_MEDIA_HEADER;
			break;
		case MediaPlayer::Stop:
			message[0] = STOP_MEDIA_HEADER;
			localAction = &TrypdeckMediaManager::stop;
			break;
		default:
			message[0] = PAUSE_MEDIA_HEADER;
			localAction = &TrypdeckMediaManager::pause;
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

void TrypdeckLeader::_triggerLedAnimationForState(TrypdeckState state, MediaPlayer::MediaPlaybackOption playbackOption) {
	TrypdeckStateChangedArgs args = { };
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

void TrypdeckLeader::_handleDigitalInput(const InputArgs& args) {
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

void TrypdeckLeader::_handleChainPull(char id) {
	bool chainPulled = false;

	if (id == LEADER_ID) {
		// TODO: Write internal set state method..
		// check if leader's chain has already been pulled
		if (_status.state == Pulled)
			return;

		chainPulled = true;

		// first chain pull -- handle internally
		_status.state = Pulled;
		_chainPulled = true;
		_onStateChanged();
	} else {
		// check if follower node's chain has already been pulled
		if (_nodeIdToStatus[id].state == Pulled)
			return;

		chainPulled = true;

		// first chain pull -- update follower status
		_nodeIdToStatus[id].state = Pulled;

		// send Pulled state update message to Followers (change led effect but not video)
		// at this point, we rely on Leader's internal representation of follower state to determine behavior
		TrypdeckStateChangedArgs args = { };
		args.state = Pulled;
		args.mediaOption = Video;
		args.playbackOption = MediaPlayer::OneShot;
		_updateStateFollower(id, args);
	}

	// add one shot action for initialize pre-reveal
	if (!_revealTriggered) {
		#if ENABLE_MEDIA_DEBUG
		// TODO: Remove debug code
		std::cout << "Starting countdown to reveal!" << std::endl;
		#endif

		_addOneShotAction(&TrypdeckLeader::_executePreReveal, PULL_TO_PRE_REVEAL_TIME);
		_revealTriggered = true;
	}

	// check if all chains have been pulled
	// skip straight to pre-reveal if true
	if (_verifyAllPulled()) {
		#if ENABLE_MEDIA_DEBUG
		// TODO: Remove debug code
		std::cout << "Removing _executePreReveal to one shot actions" << std::endl;
		#endif

		_cancelOneShotActions();
		_executePreReveal();
		return;
	}

	if (chainPulled) {
		#if ENABLE_MEDIA_DEBUG
		// TODO: Remove debug code
		std::cout << "Chain pulled!  Node with id: " << id << ". Updating node" << std::endl;
		#endif
	}

	if (_firstPull) {
		#if LED_ANIMATION_EACH_PULL
		_triggerLedAnimationForState(Pulled);
		#endif

		_firstPull = false;
	}
}

void TrypdeckLeader::_handleReset() {
	// TODO
	// careful...
}

void TrypdeckLeader::_handleShutdown() {
	_serial->transmit(std::string(SYSTEM_SHUTDOWN_MESSAGE));
	_shutdown();
}

void TrypdeckLeader::_handleMediaPlayerPlaybackComplete(const TrypdeckStateChangedArgs& args) {
	// currently only called when Led && Cycle
	if (_status.state == Wait && args.state == Wait) {
		int64_t currentTime = Clock::instance().millis();
		if (currentTime > _lastPlaybackCompleteMessageMillis + PLAYBACK_MESSAGE_WAIT_INTERVAL) {
			_lastPlaybackCompleteMessageMillis = currentTime;

			// try twice to get random led animation:
			TrypdeckStateChangedArgs newArgs = args;
			newArgs.ledId = _mediaManager->getRandomLedId(_status.state);

			if (newArgs.ledId == args.ledId)
				newArgs.ledId = _mediaManager->getRandomLedId(_status.state);
			
			std::string message = _populateBufferFromStateArgs(newArgs, PLAY_MEDIA_FROM_ARGS_HEADER);

			#if ENABLE_MEDIA_DEBUG
			// TODO: Remove debug code
			std::cout << "Cycling to next led animation for state " << _status.state << ": " << Index::instance().getSystemPath(newArgs.ledId);
			#endif

			for (const auto& pair : _nodeIdToStatus) {
				message[ID_INDEX] = pair.first;
				_serial->transmit(message);
			}

			_mediaManager->updateStateLed(newArgs);
		}
	}
}

void TrypdeckLeader::_executePreReveal() {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Executing pre reveal!" << std::endl;
	#endif

	#if PRE_REVEAL_TO_REVEAL_TIME
	// if we are using pre-reveal state, stop all media and add actual reveal to one shot actions
	_updateMediaStateUniversal(Both, MediaPlayer::Stop);
	_addOneShotAction(&TrypdeckLeader::_executeReveal, PRE_REVEAL_TO_REVEAL_TIME);
	#else
	//trigger reveal right away
	_updateMediaStateUniversal(Both, MediaPlayer::Stop);
	_executeReveal();
	#endif

	// set next action, reveal
}

void TrypdeckLeader::_executeReveal() {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Executing reveal!" << std::endl;
	#endif

	// notify all nodes to play video and led animation
	uint32_t ledId = _mediaManager->getRandomLedId(Reveal);

	for (auto const& pair : _nodeIdToStatus) {
		TrypdeckStateChangedArgs args = { };
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

	_addOneShotAction(&TrypdeckLeader::_returnToWait, REVEAL_TIME);
}

void TrypdeckLeader::_returnToWait() {
	_revealTriggered = false;
	_chainPulled = false;
	_firstPull = true;
	_status.state = Wait;
	_onStateChanged();
}