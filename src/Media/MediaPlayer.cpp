#include "MediaPlayer.h"

MediaPlayer::MediaPlayer() { }

MediaPlayer::~MediaPlayer() {
	delete _playbackCompleteDelegate;
}

MediaPlayer::MediaPlayerState MediaPlayer::getState() {
	return _state;
}

bool MediaPlayer::isPlaying() {
	return _state == MediaPlayerState::Play;
}

void MediaPlayer::setPlaybackCompleteDelegate(Command* delegate) {
	_playbackCompleteDelegate = delegate;
}

void MediaPlayer::_onPlaybackComplete() {
	if (_playbackCompleteDelegate) {
		PlaybackCompleteArgs args = { _currentOption, _currentMedia };
		_playbackCompleteDelegate->execute((CommandArgs)&args);
	}
}