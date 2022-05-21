#include "settings.h"
#include "td_utility.h"

const std::string td_utility::getVideoPath(const std::string& path) {
	return VIDEO_DIRECTORY + path;
}

const std::string td_utility::getAnimPath(const std::string& path) {
	return LED_ANIMATION_DIRECTORY + path;
}