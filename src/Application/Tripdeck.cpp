#include <iostream>
#include <iomanip>

#include "Tripdeck.h"

const char ValidHeaders[] = { STARTUP_NOTIFICATION_HEADER, STATE_CHANGED_HEADER, STATUS_UPDATE_HEADER, CONNECTION_CONFIRMATION_HEADER, PLAY_MEDIA_HEADER, STOP_MEDIA_HEADER, PAUSE_MEDIA_HEADER };

// tripdeck behavior
Tripdeck::Tripdeck(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : _mediaManager(mediaManager), _inputManager(inputManager), _serial(serial) { }

void Tripdeck::init() {
	// initialize members
	_inputManager->init();
	_mediaManager->init();
	_serial->init();
	
	// create and add serial input and serial input delegate to InputManager
	// ID does not matter in this case -- objects will be cleaned up by InputManger on Destruction
	_inputManager->addInput(new InputThreadedSerial((char)0xFF, _serial), new SerialInputDelegate(this));


	
	_status.state = Connecting;
	_run = true;
}

void Tripdeck::run() {
	_inputManager->run();
	_mediaManager->run();
}

bool Tripdeck::_validateSerialMessage(const std::string& buffer) {
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

bool Tripdeck::_validateHeader(char header) {
	for (char h : ValidHeaders) {
		if (header == h)
			return true;
	}
	return false;
}

Tripdeck::MediaHashes Tripdeck::_parseMediaHashes(const std::string& buffer) {
	std::string mediaHashes = buffer.substr(HASH_INDEX);
	int32_t slashIndex = mediaHashes.find("/");
	return MediaHashes { std::stoul(mediaHashes.substr(0, slashIndex), NULL, 16), std::stoul(mediaHashes.substr(slashIndex + 1), NULL, 16) };
}

const std::string Tripdeck::_hashToHexString(uint32_t hash) {
	std::stringstream stream;
	stream << std::hex << hash;
	return stream.str();
}

// Serial Input Delegate
Tripdeck::SerialInputDelegate::SerialInputDelegate(Tripdeck* owner) : _owner(owner) { }

Tripdeck::SerialInputDelegate::~SerialInputDelegate() { }

void Tripdeck::SerialInputDelegate::execute(CommandArgs args) {
	_owner->_handleSerialInput(*((InputArgs*)args));
}
