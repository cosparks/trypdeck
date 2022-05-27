#include <iostream>

#include "VideoPlayer.h"
#include "td_util.h"

VideoPlayer::VideoPlayer(const std::vector<std::string>& folders) {
	_mediaFolders = folders;
}

VideoPlayer::~VideoPlayer() { }

void VideoPlayer::init(const std::vector<std::string>& movies, const char *const *argv, int argc) {
	_instance = libvlc_new(argc, argv);

	// generate (sorted) media cache for all video files
	for (std::string movie : movies)
		_mediaCache[movie] = libvlc_media_new_path(_instance, td_util::getVideoPath(movie).c_str());

	// create sorted media list from cached media descriptors
	_mediaList = libvlc_media_list_new(_instance);
	int32_t i = 0;
	for (auto const& pair : _mediaCache) {
		std::cout << "inserting media for file: " << pair.first << std::endl;
		if (_insertMedia(pair.second, i++) < 0)
			throw std::runtime_error("Error: unable to insert media descriptor into media list");
	}

	_mediaListPlayer = libvlc_media_list_player_new(_instance);
	libvlc_media_list_player_set_media_list(_mediaListPlayer, _mediaList);
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

void VideoPlayer::updateMedia(MediaChangedArgs args) {

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

const std::vector<std::string>& VideoPlayer::getMediaFolders() {
	return _mediaFolders;
}

int32_t VideoPlayer::_insertMedia(libvlc_media_t* media, int32_t i) {
	libvlc_media_list_lock(_mediaList);
	int32_t ret = libvlc_media_list_insert_media(_mediaList, media, i);
	libvlc_media_list_unlock(_mediaList);
	return ret;
}