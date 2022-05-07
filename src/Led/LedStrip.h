#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#include <string>
#include "ws2811/ws2811.h"

/**
 * @brief wrapper for ws2811 struct
 */
class LedStrip {
	public:
		typedef ws2811_return_t LedStripStatus;
		LedStrip();
		~LedStrip();
		LedStripStatus init();
		LedStripStatus render();
		LedStripStatus wait();
		static const char* getStatusString(LedStripStatus status);
		void setCustomGammaFactor(double gamma_factor);
	private:
		ws2811_t _ws2811;
};

#endif