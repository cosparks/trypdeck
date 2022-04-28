#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <chrono>

using namespace std::chrono;

/**
 * @brief Simple time library for this project
 * Note: this class is a singleton so you can't call the constructor directly
 */
class Clock {
	public:
		static Clock& instance();
		Clock();
		~Clock();
		void reset();
		int64_t millis();
		int64_t seconds();
	private:
		static Clock* _instance;
		int64_t _startTime;
		int64_t _currentTimeInMilliseconds();
};

#endif