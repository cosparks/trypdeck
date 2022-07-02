
#include "VideoPlayerOmx.h"

#include <iostream>
#include "settings.h"
#include "td_util.h"
#include "Clock.h"
#include "Index.h"

#define THREAD_SLEEP_MICROS 200

#define KILL_COMMAND "killall omxplayer.bin"
#define ONESHOT_ARGS "omxplayer --no-osd "
#define LOOP_ARGS "omxplayer --loop --no-osd "

VideoPlayerOmx::VideoPlayerOmx() { }

VideoPlayerOmx::~VideoPlayerOmx() {
	if (_isPlaying)
		system(KILL_COMMAND);
}

void VideoPlayerOmx::init() {
	stop();
}

void VideoPlayerOmx::setCurrentMedia(uint32_t fileId, MediaPlaybackOption option) {
	if (_fileIdToSystemPath.find(fileId) == _fileIdToSystemPath.end()) {
		#if ENABLE_MEDIA_DEBUG
		// TODO: Remove debug code
		std::cout << "VideoPlayerOmx: media not found!" << std::endl;
		#endif

		return;
	}

	_currentMedia = fileId;
	_currentOption = option;

	_stateMutex.lock();
	_omxplayerArgs = (option == MediaPlaybackOption::OneShot ? ONESHOT_ARGS : LOOP_ARGS) + _fileIdToSystemPath[fileId];
	_stateMutex.unlock();
}

uint32_t VideoPlayerOmx::getCurrentMedia() {
	return _currentMedia;
}

uint32_t VideoPlayerOmx::getNumMediaFiles() {
	return _fileIdToSystemPath.size();
}

void VideoPlayerOmx::play() {
	bool yield = false;

	// lock to read _isPlaying
	_stateMutex.lock();
	if (_isPlaying) {
		_stateMutex.unlock();

		stop();
		yield = true;
	} else {
		_stateMutex.unlock();
	}

	// yield to allow other thread to stop
	if (yield)
		std::this_thread::sleep_for(std::chrono::microseconds(THREAD_SLEEP_MICROS));

	std::thread player(&VideoPlayerOmx::_playInternal, this);
	player.detach();
}

void VideoPlayerOmx::stop() {
	_stateMutex.lock();
	if (_isPlaying) {
		_isPlaying = false;
		_isPlayingOneShot = false;
	}
	_stateMutex.unlock();

		#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	int64_t before = Clock::instance().micros();
	#endif
	
	system(KILL_COMMAND);

	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	int64_t after = Clock::instance().micros();
	std::cout << "'killall omxplayer.bin' execution time: " << after - before << std::endl;
	#endif
}

void VideoPlayerOmx::pause() {
	// pause not possible with omx player
}

bool VideoPlayerOmx::containsMedia(uint32_t fileId) {
	return _fileIdToSystemPath.find(fileId) != _fileIdToSystemPath.end();
}

// uint32_t _currentMedia;
// bool isPlaying;
// std::unordered_map<uint32_t, std::string> _fileIdToSystemPath;

void VideoPlayerOmx::_addMedia(uint32_t fileId) {
	if (_fileIdToSystemPath.find(fileId) == _fileIdToSystemPath.end())
		_fileIdToSystemPath[fileId] = Index::instance().getSystemPath(fileId);
}

void VideoPlayerOmx::_removeMedia(uint32_t fileId) {
	if (_fileIdToSystemPath.find(fileId) != _fileIdToSystemPath.end())
		_fileIdToSystemPath.erase(fileId);
}

void VideoPlayerOmx::_updateMedia(uint32_t fileId) {
	// lock to read _isPlaying
	_stateMutex.lock();
	if (_isPlaying && _currentMedia == fileId) {
		_stateMutex.unlock();

		stop();
		play();
	} else {
		_stateMutex.unlock();
	}
}

void VideoPlayerOmx::_playInternal() {
	// set thread exit callback
	
	// lock to modify state and call on thread exit
	_stateMutex.lock();
	std::string playerArgs;
	_isPlaying = true;
	_isPlayingOneShot = _currentOption == OneShot;
	playerArgs = _omxplayerArgs;
	td_util::onThreadExit<VideoPlayerOmx>(this, &VideoPlayerOmx::_threadExitCallback);
	_stateMutex.unlock();

	system(playerArgs.c_str());
}

void VideoPlayerOmx::_threadExitCallback() {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Omx player thread has exited" << std::endl;
	#endif

	_stateMutex.lock();
	if (_isPlayingOneShot)
		_isPlaying = false;
	_stateMutex.unlock();
}
