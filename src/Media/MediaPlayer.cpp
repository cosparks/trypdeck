#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(const std::vector<std::string>& folders) : MediaListener(folders) { }

MediaPlayer::~MediaPlayer() { }

MediaPlayer::MediaPlayerState MediaPlayer::getState() {
	return _state;
}