#include <iostream>

#include "TripdeckFollower.h"

TripdeckFollower::TripdeckFollower(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : Tripdeck(mediaManager, inputManager, serial) { }

TripdeckFollower::~TripdeckFollower() { }

void TripdeckFollower::init() {
	// sets state to Connecting and _run to true
	Tripdeck::init();

	TripdeckStateChangedArgs args = { };
	args.state = _status.state;
	args.mediaOption = Both;
	args.playbackOption = MediaPlayer::Loop;
	_onStateChanged(args);
}

void TripdeckFollower::run() {
	while (_run) {
		Tripdeck::run();

		switch (_status.state) {
			case Connecting:
				_runTimedAction(this, &TripdeckFollower::_pingLeader);
				break;
			case Connected:
				// behavior is same as Wait
			case Wait:
				_runTimedAction(this, &TripdeckFollower::_sendStatusUpdate);
				break;
			case Pulled:
				// followers and leaders may be out of sync during Pulled phase
				break;
			case Reveal:
				_runTimedAction(this, &TripdeckFollower::_sendStatusUpdate);
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

void TripdeckFollower::_pingLeader() {
	std::string data(HEADER_LENGTH, STARTUP_NOTIFICATION_HEADER);
	data += ID;
	_serial->transmit(data);
}

void TripdeckFollower::_sendStatusUpdate() {
	std::string message = DEFAULT_MESSAGE;
	message[0] = STATUS_UPDATE_HEADER;
	message[ID_INDEX] = ID;
	message[STATE_INDEX] = _singleDigitIntToChar((int32_t)_status.state);
	message[MEDIA_OPTION_INDEX] = _singleDigitIntToChar((int32_t)_status.option);

	_serial->transmit(message);
}

void TripdeckFollower::_handleSerialInput(const InputArgs& args) {
	if (!_validateSerialMessage(args.buffer))
		return;
	
	// check if message is intended for this follower
	if (_parseId(args.buffer)== ID) {
		
		char header = _parseHeader(args.buffer);
		TripdeckMediaOption mediaOption = None;

		switch (header) {
			case STATE_CHANGED_HEADER:
				_handleStateChangedMessage(args.buffer);
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
			case PLAY_MEDIA_FROM_ARGS_HEADER:
				_handlePlayMediaFromArgsMessage(args.buffer);
				break;
			case SYSTEM_RESET_HEADER:
				_reset();
				break;
			case SYSTEM_SHUTDOWN_HEADER:
				_shutdown();
				break;
			default:
				// do nothing
				break;
		}
	} else {
		// if transmission is not for us, pass it on
		_serial->transmit(args.buffer);
	}
}

void TripdeckFollower::_handleStateChangedMessage(const std::string& buffer) {
	TripdeckStateChangedArgs stateArgs = { };

	if (_parseStateChangedMessage(buffer, stateArgs)) {
		_updateStatusFromStateArgs(stateArgs);
		_onStateChanged(stateArgs);
	}
}

void TripdeckFollower::_handlePlayMediaFromArgsMessage(const std::string& buffer) {
	TripdeckStateChangedArgs stateArgs = { };
	_populateStateArgsFromBuffer(buffer, stateArgs);

	switch (stateArgs.mediaOption) {
		case None:
			// do nothing
			break;
		case Video:
			_mediaManager->updateStateVideo(stateArgs);
			break;
		case Led:
			_mediaManager->updateStateLed(stateArgs);
			break;
		default: // Both
			_mediaManager->updateState(stateArgs);
			break;
	}
}

// returns true and loads StateChangedArgs if entering new state, false otherwise
bool TripdeckFollower::_parseStateChangedMessage(const std::string& buffer, TripdeckStateChangedArgs& args) {
	if (buffer.length() < 6)
		throw std::runtime_error("Error: Invalid state changed message.  State message length must be >= 6");

	// parse message data only if we are entering a new state
	if (_parseState(buffer) != _status.state) {
		_populateStateArgsFromBuffer(buffer, args);
		return true;
	}
	return false;
}

void TripdeckFollower::_handleMediaPlayerPlaybackComplete(const TripdeckStateChangedArgs& args) {
	// currently only called when Wait && Led && Cycle
	if (_status.state == Wait) {
		std::string message = _populateBufferFromStateArgs(args, MEDIA_PLAYBACK_COMPLETE_HEADER, ID);
		_serial->transmit(message);
	}
}