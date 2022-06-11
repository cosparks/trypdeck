#include "MediaPlayer.h"

MediaPlayer::MediaPlayer() { }

MediaPlayer::~MediaPlayer() { }

MediaPlayer::MediaPlayerState MediaPlayer::getState() {
	return _state;
}

bool MediaPlayer::isPlaying() {
	return _state == MediaPlayerState::Play;
}