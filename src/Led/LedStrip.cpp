#include "LedStrip.h"

LedStrip::LedStrip() { }
LedStrip::~LedStrip() { }

LedStrip::LedStripStatus LedStrip::init() {
	return ws2811_init(&_ws2811);
}

LedStrip::LedStripStatus LedStrip::render() {
	return ws2811_render(&_ws2811);
}

LedStrip::LedStripStatus LedStrip::wait() {
	return ws2811_wait(&_ws2811);
}

const char* LedStrip::getStatusString(LedStrip::LedStripStatus status) {
	return ws2811_get_return_t_str(status);
}

void LedStrip::setCustomGammaFactor(double gamma_factor) {
	ws2811_set_custom_gamma_factor(&_ws2811, gamma_factor);
}