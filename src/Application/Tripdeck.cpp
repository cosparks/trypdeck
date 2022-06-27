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
	
	_currentState = Connecting;
	_run = true;
}

void Tripdeck::run() {
	_inputManager->run();
	_mediaManager->run();
}

void Tripdeck::setStateChangedDelegate(Command* delegate) {
	_stateChangedDelegate = delegate;
}

// serial input delegate
Tripdeck::SerialInputDelegate::SerialInputDelegate(Tripdeck* owner) : _owner(owner) { }

Tripdeck::SerialInputDelegate::~SerialInputDelegate() { }

void Tripdeck::SerialInputDelegate::execute(CommandArgs args) {
	_owner->_handleSerialInput(*((InputArgs*)args));
}
