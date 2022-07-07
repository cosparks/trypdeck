#ifndef _INPUT_DIGITAL_BUTTON_H_
#define _INPUT_DIGITAL_BUTTON_H_

#include "td_util.h"

using namespace td_util;

class InputDigitalButton : public Input {
	public:
		InputDigitalButton(char id, uint32_t gpio);
		~InputDigitalButton();
		bool read() override;
	private:
		uint32_t _gpio;
		int64_t _lastReadHigh = 0;
};

#endif