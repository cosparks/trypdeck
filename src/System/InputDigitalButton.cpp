#include "InputDigitalButton.h"

#include <iostream>
#include <stdexcept>
#include <pigpio.h>
#include "settings.h"
#include "Clock.h"

InputDigitalButton::InputDigitalButton(char id, uint32_t gpio) : Input(id), _gpio(gpio) {
	_data = "1";
}

InputDigitalButton::~InputDigitalButton() { }

bool InputDigitalButton::read() {
	int64_t currentTime = Clock::instance().millis();

	if (currentTime > _lastReadHigh + BUTTON_RESET_INTERVAL_MILLIS) {
		int32_t ret = gpioRead(_gpio);

		if (ret == PI_BAD_GPIO) {
			throw std::runtime_error("Error: digitalRead() in InputDigitalButton::read() returned PI_BAD_GPIO");
		}

		if (ret == 1) {
			#if ENABLE_SERIAL_DEBUG
			// TODO: Remove debug code
			std::cout << "Read HIGH from button: " << _id << " -- gpio pin: " << _gpio << std::endl;
			#endif
			
			_lastReadHigh = currentTime;
		}

		return ret == 1;
	}

	return false;
}
