#include "settings.h"

#include "td_util.h"

const std::string td_util::getVideoPath(const std::string& path) {
	return CARD_VIDEO_DIRECTORY + path;
}

const std::string td_util::getAnimPath(const std::string& path) {
	return LED_ANIMATION_DIRECTORY + path;
}