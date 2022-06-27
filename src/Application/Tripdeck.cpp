#include "Tripdeck.h"

// tripdeck behavior
Tripdeck::Tripdeck(InputManager* inputManager, Serial* serial) : _inputManager(inputManager), _serial(serial) { }

void Tripdeck::init() {
	_inputManager->init();
	_serial->init();

	_serialInput = new InputThreadedSerial(5, _serial);
	_serialInputDelegate = new SerialInputDelegate(this);
	_inputManager->addInput(_serialInput, _serialInputDelegate);
	_currentState = Connecting;
}

void Tripdeck::run() {
	_inputManager->run();
}

Tripdeck::TripdeckState Tripdeck::getState() {
	return _currentState;
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
