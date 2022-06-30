#include "InputManager.h"

InputManager::InputManager() { }

InputManager::~InputManager() {
	for (const auto& pair : _inputToCommand) {
		delete pair.first;
		delete pair.second;
	}
}

void InputManager::init() { }

void InputManager::run() {
	for (const auto& pair : _inputToCommand) {
		if (pair.first->read()) {
			InputArgs data = { };
			data.id = pair.first->getId();
			data.buffer = pair.first->getData();
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
