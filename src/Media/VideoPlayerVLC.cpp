#include <iostream>

#include "settings.h"
#include "VideoPlayerVLC.h"
#include "Index.h"

#if ENABLE_VLC_DEBUG
const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };
#define VLC_NUM_ARGS 9
#else
const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--quiet", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };
#define VLC_NUM_ARGS 10
#endif

VideoPlayerVLC::VideoPlayerVLC() { }

VideoPlayerVLC::~VideoPlayerVLC() {
	libvlc_media_list_release(_mediaList);
	libvlc_media_list_player_release(_mediaListPlayer);
	libvlc_release(_instance);
}

void VideoPlayerVLC::init() {
	_instance = libvlc_new(VLC_NUM_ARGS, VLC_ARGS);
	_mediaList = libvlc_media_list_new(_instance);
	_mediaListPlayer = libvlc_media_list_player_new(_instance);
	libvlc_media_list_player_set_media_list(_mediaListPlayer, _mediaList);
}

void VideoPlayerVLC::run() { }

void VideoPlayerVLC::setCurrentMedia(uint32_t fileId, MediaPlaybackOption option) {
	if (_fileIdToIndex.find(fileId) == _fileIdToIndex.end()) {
		throw std::runtime_error("Media not found: cache does not contain the media requested");
	}

	switch (option) {
		case MediaPlaybackOption::OneShot:
			libvlc_media_list_player_set_playback_mode(_mediaListPlayer, libvlc_playback_mode_default);
			break;
		case MediaPlaybackOption::Loop:
			libvlc_media_list_player_set_playback_mode(_mediaListPlayer, libvlc_playback_mode_repeat);
			break;
		default:
			break;
	}

	_currentMedia = fileId;
}

uint32_t VideoPlayerVLC::getCurrentMedia() {
	return _currentMedia;
}

uint32_t VideoPlayerVLC::getNumMediaFiles() {
	return _fileIdToIndex.size();
}

void VideoPlayerVLC::play() {
	if (_mediaListPlayer == NULL) {
		throw std::runtime_error("Error: VideoPlayerVLC has not been initialized!");
	}

	if (!_currentMedia) {
		throw std::runtime_error("Error: VideoPlayerVLC::_currentMedia has either not been set, or has been removed!");
	}

	if (libvlc_media_list_player_is_playing(_mediaListPlayer))
		stop();

	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "VLC: playing media file " << Index::instance().getSystemPath(_currentMedia) << std::endl;
	#endif

	_state = MediaPlayerState::Play;
	libvlc_media_list_player_play_item_at_index(_mediaListPlayer, _fileIdToIndex[_currentMedia]);


	// // TODO: Remove debug code
	// std::cout << "VideoPlayerVLC::play() called with file: " << Index::instance().getSystemPath(_currentMedia) << std::endl;
}

void VideoPlayerVLC::stop() {
	if (_state == Stop)
		return;
	
	_state = MediaPlayerState::Stop;
	libvlc_media_list_player_stop(_mediaListPlayer);
}

void VideoPlayerVLC::pause() {
	if (_state == Pause)
		return;

	_state = MediaPlayerState::Pause;
	libvlc_media_list_player_pause(_mediaListPlayer);
}

bool VideoPlayerVLC::containsMedia(uint32_t fileId) {
	return _fileIdToIndex.find(fileId) != _fileIdToIndex.end();
}

void VideoPlayerVLC::_addMedia(uint32_t fileId) {
	if (_fileIdToIndex.find(fileId) == _fileIdToIndex.end()) {
		int32_t i = 0;

		// reuse empty indices in media list if any are available
		// TODO: Investigate bug related to overwriting media items in media list (causes playback issue)
		if (false && _emptyIndices.size() > 0) {
			i = _emptyIndices.front();
			_emptyIndices.pop();
		} else {
			i = _mediaListSize;
			_mediaListSize++;
		}

		_fileIdToIndex[fileId] = i;
		_createAndInsertMedia(fileId, i);
	}

	#if PLAY_MEDIA_ON_ADD
	// // TODO: Remove temp behavior for testing
	if (libvlc_media_list_player_is_playing(_mediaListPlayer)) {
		setCurrentMedia(fileId, MediaPlaybackOption::Loop);

		if (_state == MediaPlayerState::Play)
			play();
	}
	#endif
}

void VideoPlayerVLC::_removeMedia(uint32_t fileId) {
	if (_fileIdToIndex.find(fileId) != _fileIdToIndex.end()) {
		if (_currentMedia == fileId) {
			_currentMedia = 0;
			stop();
		}

		int32_t i = _fileIdToIndex[fileId];
		_fileIdToIndex.erase(fileId);
		_removeMediaAtIndex(i);
		_emptyIndices.emplace(i);
	}
}

void VideoPlayerVLC::_updateMedia(uint32_t fileId) {
	if (_fileIdToIndex.find(fileId) != _fileIdToIndex.end()) {
		int32_t i = _fileIdToIndex[fileId];
		_removeMediaAtIndex(i);
		_createAndInsertMedia(fileId, i);
	}
	else {
		_addMedia(fileId);
	}
}

void VideoPlayerVLC::_createAndInsertMedia(uint32_t fileId, int32_t i) {
	libvlc_media_t* media = libvlc_media_new_path(_instance, Index::instance().getSystemPath(fileId).c_str());

	libvlc_media_list_lock(_mediaList);
	int32_t ret = libvlc_media_list_insert_media(_mediaList, media, i);
	libvlc_media_list_unlock(_mediaList);

	// decrement ref count of media
	libvlc_media_release(media);

	if (ret == -1)
		throw std::runtime_error("Error: Unable to insert media.  Media list is read only");
}

void VideoPlayerVLC::_removeMediaAtIndex(int32_t i) {
	libvlc_media_list_lock(_mediaList);
	libvlc_media_list_remove_index(_mediaList, i);
	libvlc_media_list_unlock(_mediaList);
}