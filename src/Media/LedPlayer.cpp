#include "LedPlayer.h"

LedPlayer::LedPlayer(const std::vector<std::string>& folders) : MediaListener(folders) { }

LedPlayer::~LedPlayer() { }

void LedPlayer::_addMedia(uint32_t fileId) {
	// TODO: implement
}

void LedPlayer::_removeMedia(uint32_t fileId) {
	// TODO: implement
}

void LedPlayer::_updateMedia(uint32_t fileId) {
	// TODO: implement
}