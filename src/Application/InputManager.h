#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include "td_util.h"

using namespace td_util;

class InputManager : public Observer {
	public:
		InputManager();
		~InputManager();
		void handleInput();
};

#endif