#include "InputManager.h"

InputManager::InputManager() { }

InputManager::~InputManager() { }

void InputManager::init() { }

void InputManager::run() {
	for (const auto& pair : _inputToCommand) {
		if (pair.first->read()) {
			InputArgs data = InputArgs { pair.first->getId(), pair.first->getData() };
			pair.second->execute((CommandArgs)&data);
		}
	}
}

void InputManager::addInput(Input* input, Command* command) {
	if (_inputToCommand.find(input) == _inputToCommand.end())
		_inputToCommand[input] = command;
}

void InputManager::removeInput(Input* input) {
	if (_inputToCommand.find(input) != _inputToCommand.end())
		_inputToCommand.erase(input);
}
