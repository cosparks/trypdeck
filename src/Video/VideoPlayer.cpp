#include "VideoPlayer.h"
#include "td_utility.h"

VideoPlayer::VideoPlayer() { }

VideoPlayer::~VideoPlayer() { }

void VideoPlayer::init(const std::vector<std::string>& movies, const char *const *argv, int argc) {
	_instance = libvlc_new(argc, argv);
	_mediaPlayer = libvlc_media_player_new(_instance);

	for (std::string movie : movies) {
		_mediaCache[movie] = libvlc_media_new_path(_instance, td_utility::getVideoPath(movie).c_str());
	}
}

void VideoPlayer::setCurrentMedia(const std::string& path, VideoPlaybackOption option) {
	if (_mediaPlayer != nullptr) {
		if (libvlc_media_player_is_playing(_mediaPlayer))
			stop();
	}

	if (_mediaCache.find(path) == _mediaCache.end()) {
		throw std::runtime_error("Media not found: cache does not contain the media requested");
	}

	_currentMedia = path;
	libvlc_media_t* media = _mediaCache[path];

	// TODO figure out a better way to do this
	if (option == Loop) {
		libvlc_media_add_option(media, "input-repeat=-1");
	}

	libvlc_media_player_set_media(_mediaPlayer, media);
}

std::string VideoPlayer::getCurrentMedia() {
	return _currentMedia;
}

void VideoPlayer::play() {
	libvlc_media_player_play(_mediaPlayer);
}

// TODO Figure out looping behavior
void VideoPlayer::playLoop() {
	libvlc_media_player_play(_mediaPlayer);
}

void VideoPlayer::stop() {
	libvlc_media_player_stop(_mediaPlayer);
}

void VideoPlayer::pause() {

}