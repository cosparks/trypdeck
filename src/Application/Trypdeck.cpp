#include <iostream>
#include <iomanip>

#include "Trypdeck.h"

const char ValidHeaders[] = { STARTUP_NOTIFICATION_HEADER, STATE_CHANGED_HEADER, STATUS_UPDATE_HEADER, PLAY_MEDIA_HEADER,
	STOP_MEDIA_HEADER, PAUSE_MEDIA_HEADER, PLAY_MEDIA_FROM_ARGS_HEADER, MEDIA_PLAYBACK_COMPLETE_HEADER, SYSTEM_RESET_HEADER, SYSTEM_SHUTDOWN_HEADER };

// trypdeck behavior
Trypdeck::Trypdeck(TrypdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : _mediaManager(mediaManager), _inputManager(inputManager), _serial(serial) { }

void Trypdeck::init() {
	// initialize members
	_inputManager->init();
	_mediaManager->init();
	_serial->init();
	
	// create and add serial input and serial input delegate to InputManager
	// ID does not matter in this case -- objects will be cleaned up by InputManger on Destruction
	_inputManager->addInput(new InputThreadedSerial((char)0xFF, _serial), new Delegate<Trypdeck, InputArgs>(this, &Trypdeck::_handleSerialInput));
	// _inputManager->addInput(new InputThreadedSerial((char)0xFF, _serial), new SerialInputDelegate(this));

	// add playback complete handler
	_mediaManager->setPlaybackCompleteDelegate(new Delegate<Trypdeck, TrypdeckStateChangedArgs>(this, &Trypdeck::_mediaManagerPlaybackComplete));

	_status.state = Connecting;
	_run = true;
}

void Trypdeck::run() {
	_inputManager->run();
	_mediaManager->run();
}

bool Trypdeck::_validateSerialMessage(const std::string& buffer) {
	// shortest possible message is HEADER_LENGTH + ID + '\n'
	if (buffer.length() < HEADER_LENGTH + 2) {
		#if ENABLE_SERIAL_DEBUG
		// TODO: Remove debug code
		std::cout << "Warning: Invalid message received -- length: " << buffer.length() << std::endl;
		#endif
		
		return false;
	}

	// check if first character is alphanumeric
	return iswalnum(buffer[0]) != 0 && _validateHeader(buffer[0]);
}

bool Trypdeck::_validateHeader(char header) {
	for (char h : ValidHeaders) {
		if (header == h)
			return true;
	}
	return false;
}

Trypdeck::MediaHashes Trypdeck::_parseMediaHashes(const std::string& buffer) {
	std::string mediaHashes = buffer.substr(HASH_INDEX);
	int32_t slashIndex = mediaHashes.find("/");
	return MediaHashes { std::stoul(mediaHashes.substr(0, slashIndex), NULL, 16), std::stoul(mediaHashes.substr(slashIndex + 1), NULL, 16) };
}

const std::string Trypdeck::_hashToHexString(uint32_t hash) {
	std::stringstream stream;
	stream << std::hex << hash;
	return stream.str();
}

void Trypdeck::_updateStatusFromStateArgs(TrypdeckStateChangedArgs& args) {
	_status.videoMedia = args.videoId;
	_status.ledMedia = args.ledId;
	_status.option = args.mediaOption;
	_status.state = args.state;
	_status.lastTransmitMillis = Clock::instance().millis();
	_status.connected = true;
}

void Trypdeck::_populateStateArgsFromBuffer(const std::string& buffer, TrypdeckStateChangedArgs& args) {
	args.state = _parseState(buffer);

	// check for media id info
	if (_containsMediaHashes(buffer)) {
		MediaHashes hashes = _parseMediaHashes(buffer);
		args.videoId = hashes.videoHash;
		args.ledId = hashes.ledHash;
	}

	args.mediaOption = _parseMediaOption(buffer);
	args.playbackOption = _parsePlaybackOption(buffer);
}

std::string Trypdeck::_populateBufferFromStateArgs(const TrypdeckStateChangedArgs& args, char header, char id) {
	std::string message = DEFAULT_MESSAGE;
	message[0] = header;
	message[ID_INDEX] = id;
	message[STATE_INDEX] = _singleDigitIntToChar((int32_t)args.state);
	message[MEDIA_OPTION_INDEX] = _singleDigitIntToChar((int32_t)args.mediaOption);
	message[PLAYBACK_OPTION_INDEX] = _singleDigitIntToChar((int32_t)args.playbackOption);
	
	if (args.videoId || args.ledId)
		message.append("/" + _hashToHexString(args.videoId) + "/" + _hashToHexString(args.ledId));

	return message;
}

void Trypdeck::_mediaManagerPlaybackComplete(const TrypdeckStateChangedArgs& args) {
	// at the moment, we only care about Cycling Led videos
	if (args.mediaOption == Led && args.playbackOption == MediaPlayer::Cycle) {
		_handleMediaPlayerPlaybackComplete(args);
	}
}

void Trypdeck::_reset() {
	system("sudo reboot");
}

void Trypdeck::_shutdown() {
	system("sudo shutdown -h now");
}

// Serial Input Delegate
Trypdeck::SerialInputDelegate::SerialInputDelegate(Trypdeck* owner) : _owner(owner) { }

Trypdeck::SerialInputDelegate::~SerialInputDelegate() { }

void Trypdeck::SerialInputDelegate::execute(CommandArgs args) {
	_owner->_handleSerialInput(*((InputArgs*)args));
}
