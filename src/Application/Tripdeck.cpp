#include <iostream>
#include <iomanip>

#include "Tripdeck.h"

// tripdeck behavior
Tripdeck::Tripdeck(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial) : _mediaManager(mediaManager), _inputManager(inputManager), _serial(serial) { }

void Tripdeck::init() {
	// initialize members
	_inputManager->init();
	_mediaManager->init();
	_serial->init();

	// create delegate to listen for serial input
	_serialInput = new InputThreadedSerial(5, _serial);
	_serialInputDelegate = new SerialInputDelegate(this);
	_inputManager->addInput(_serialInput, _serialInputDelegate);
	
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
		
		return true;
	}
	
	return false;
}


Tripdeck::MediaHashes Tripdeck::_parseMediaHashes(const std::string& buffer) {
	std::string mediaHashes = buffer.substr(HEADER_LENGTH + 6);
	int32_t slashIndex = mediaHashes.find("/");
	return MediaHashes { std::stoul(mediaHashes.substr(0, slashIndex), NULL, 16), std::stoul(mediaHashes.substr(slashIndex + 1), NULL, 16) };
}

const std::string Tripdeck::_hashToHexString(uint32_t hash) {
	std::stringstream stream;
	stream << std::hex << hash;
	return stream.str();
}

void Tripdeck::setStateChangedDelegate(Command* delegate) {
	_stateChangedDelegate = delegate;
}

void Tripdeck::SerialInputDelegate::execute(CommandArgs args) {
	_owner->_handleSerialInput(*((InputArgs*)args));
}

// serial input delegate
Tripdeck::SerialInputDelegate::SerialInputDelegate(Tripdeck* owner) : _owner(owner) { }

Tripdeck::SerialInputDelegate::~SerialInputDelegate() { }