#ifndef _MOCK_BUTTON_H_
#define _MOCK_BUTTON_H_

#include <iostream>
#include <cstdlib>

#include "td_util.h"
#include "Clock.h"

using namespace td_util;

class MockButton : public Input {
	public:
		MockButton(char id, int64_t minRange, int64_t maxRange) : Input(id) {
			_minRange = minRange;
			_maxRange = maxRange;
		}

		~MockButton() { }

		bool read() override {
			if (Clock::instance().millis() > _nextTime) {
				int64_t random = std::rand() % _maxRange;

				if (random < _minRange)
					random = std::rand() % (_maxRange - random) + random;
				_nextTime = Clock::instance().millis() + random;

				_data = random % 2 ? "1" : "0";
				return true;
			}
			return false;
		}

	private:
		int64_t _lastTime = 0;
		int64_t _nextTime = 25000;
		int64_t _minRange = 0;
		int64_t _maxRange = 0;
};

#endif