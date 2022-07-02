#ifndef _TD_PIGPIO_H_
#define _TD_PIGPIO_H_

#include <stdexcept>
#include <pigpio.h>

class td_pigpio {
	public:
		td_pigpio() {
			#if not ENABLE_VISUAL_DEBUG
			if (gpioInitialise() < 0)
				throw std::runtime_error("Error: PI GPIO Initialization failed");
			#endif
		}

		~td_pigpio() {
			#if not ENABLE_VISUAL_DEBUG
			gpioTerminate();
			#endif
		}
};

#endif