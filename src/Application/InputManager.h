#ifndef _INPUT_MANAGER_H_
#define _INPUT_MANAGER_H_

#include "Observer.h"

class InputManager : public Observer {
	public:
		InputManager();
		~InputManager();
		void handleInput();
};

#endif