#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include <unordered_map>

#include "td_util.h"
#include "Runnable.h"

using namespace td_util;

class InputManager : public Runnable {
	public:
		InputManager();
		~InputManager();
		void init() override;
		void run() override;
		void addInput(Input* input, Command* command);
		void removeInput(Input* input);
	private:
		std::unordered_map<Input*, Command*> _inputToCommand;
};

#endif