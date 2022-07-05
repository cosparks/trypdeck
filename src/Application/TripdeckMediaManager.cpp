#include <stdexcept>
#include <iostream>
#include <thread>
#include <cstdlib>

#include "Index.h"
#include "Clock.h"
#include "TripdeckMediaManager.h"
#include "MediaListenerDummy.h"

#define RUN_TIME_MILLIS 330000000
#define VIDEO_PLAY_INTERVAL 10000
#define VLC_DELAY 600

TripdeckMediaManager::TripdeckMediaManager(DataManager* dataManager, MediaPlayer* videoPlayer, MediaPlayer* ledPlayer) :
	_dataManager(dataManager), _videoPlayer(videoPlayer), _ledPlayer(ledPlayer) {
	if (!_dataManager)
		throw std::runtime_error("Error: TripdeckMediaManager::_dataManager cannot be null!");

	_runnableObjects.push_back(_dataManager);

	if (_ledPlayer)
		_runnableObjects.push_back(_ledPlayer);
	if (_videoPlayer)
		_runnableObjects.push_back(_videoPlayer);

	if (!_ledPlayer || !_videoPlayer) {
		_mediaListener = new MediaListenerDummy();
	}
}

TripdeckMediaManager::~TripdeckMediaManager() {
	if (_playbackCompleteDelegate)
		delete _playbackCompleteDelegate;

	if (_mediaListener)
		delete _mediaListener;
}

void TripdeckMediaManager::init() {
	// initialize all runnable objects
	for (Runnable* runnable : _runnableObjects) {
		runnable->init();
	}

	// hook up media players to data manager
	if (_ledPlayer) {
		_dataManager->addMediaListener(_ledPlayer);
		_ledPlayer->setPlaybackCompleteDelegate(
			new Delegate<TripdeckMediaManager, MediaPlayer::PlaybackCompleteArgs>(this, &TripdeckMediaManager::_ledPlayerPlaybackComplete));
	}

	if (_videoPlayer) {
		_dataManager->addMediaListener(_videoPlayer);
		_videoPlayer->setPlaybackCompleteDelegate(
			new Delegate<TripdeckMediaManager, MediaPlayer::PlaybackCompleteArgs>(this, &TripdeckMediaManager::_videoPlayerPlaybackComplete));
	}

	if (_mediaListener)
		_dataManager->addMediaListener(_mediaListener);
}

void TripdeckMediaManager::run() {
	for (Runnable* runnable : _runnableObjects) {
			runnable->run();
	}
}

void TripdeckMediaManager::play(TripdeckMediaOption option) {
	switch (option) {
		case None:
			// do nothing
			break;
		case Video:
			_playVideoInternal();
			break;
		case Led:
			_playLedInternal();
			break;
		default: // Both
			_playVideoInternal();
			_playLedInternal();
			break;
	} 
}

void TripdeckMediaManager::stop(TripdeckMediaOption option) {
	switch (option) {
		case None:
			// do nothing
			break;
		case Video:
			_stopVideoInternal();
			break;
		case Led:
			_stopLedInternal();
			break;
		default: // Both
			_stopVideoInternal();
			_stopLedInternal();
			break;
	}
}

void TripdeckMediaManager::pause(TripdeckMediaOption option) {
	switch (option) {
		case None:
			// do nothing
			break;
		case Video:
			_pauseVideoInternal();
			break;
		case Led:
			_pauseLedInternal();
			break;
		default: // Both
			_pauseVideoInternal();
			_pauseLedInternal();
			break;
	}
}

void TripdeckMediaManager::updateState(const TripdeckStateChangedArgs& args) {
	if (args.state != Unknown)
		_currentState = args.state;

	updateStateVideo(args);
	updateStateLed(args);
}

void TripdeckMediaManager::updateStateVideo(const TripdeckStateChangedArgs& args) {
	if (_videoPlayer) {
		uint32_t videoId = 0;

		if (args.videoId == 0 || !_videoPlayer->containsMedia(args.videoId))
			videoId = getRandomVideoId(_currentState);
		else
			videoId = args.videoId;
		
		_videoPlayer->setCurrentMedia(videoId, (args.playbackOption == MediaPlayer::Cycle) ? MediaPlayer::Loop : args.playbackOption);

		if (args.mediaOption == Video || args.mediaOption == Both) {
			#if ENABLE_MEDIA_DEBUG
			// TODO: Remove debug code
			std::cout << "Playing video with hash: " << videoId << endl;
			#endif

			_videoPlayer->play();
		}
	}
}

void TripdeckMediaManager::updateStateLed(const TripdeckStateChangedArgs& args) {
	if (_ledPlayer) {
		uint32_t ledId = 0;

		if (args.ledId == 0 || !_ledPlayer->containsMedia(args.ledId))
			ledId = getRandomLedId(_currentState);
		else
			ledId = args.ledId;

		_ledPlayer->setCurrentMedia(ledId, args.playbackOption);

		if (args.mediaOption == Led || args.mediaOption == Both) {
			#if ENABLE_MEDIA_DEBUG
			// TODO: Remove debug code
			std::cout << "Playing led animation with hash: " << ledId << endl;
			#endif

			_ledPlayer->play();
		}
	}
}


void TripdeckMediaManager::addVideoFolder(TripdeckState state, const char* folder) {
	if (_videoPlayer) {
		_videoPlayer->addMediaFolder(folder);
	} else {
		_mediaListener->addMediaFolder(folder);
	}

	_stateToVideoFolder[state] = folder;
}

void TripdeckMediaManager::addLedFolder(TripdeckState state, const char* folder) {
	if (_ledPlayer) {
		_ledPlayer->addMediaFolder(folder);
	} else {
		_mediaListener->addMediaFolder(folder);
	}

	_stateToLedFolder[state] = folder;
}

uint32_t TripdeckMediaManager::getRandomVideoId(TripdeckState state) {
	const auto& videoFiles = _dataManager->getFileIdsFromFolder(_stateToVideoFolder[state]);

	if (videoFiles.size() == 0) {
		std::string message = "Error: video folder for current state (" + std::to_string(_currentState) + ") does not contain any files";
		throw std::runtime_error(message);
	}

	srand((uint32_t)Clock::instance().millis());
	return videoFiles[rand() % videoFiles.size()];
}

uint32_t TripdeckMediaManager::getRandomLedId(TripdeckState state) {
	const auto& ledFiles = _dataManager->getFileIdsFromFolder(_stateToLedFolder[_currentState]);

	if (ledFiles.size() == 0) {
		std::string message = "Error: led folder for current state (" + std::to_string(_currentState) + ") does not contain any files";
		throw std::runtime_error(message);
	}

	srand((uint32_t)Clock::instance().millis());
	return ledFiles[rand() % ledFiles.size()];
}

void TripdeckMediaManager::setPlaybackCompleteDelegate(Command* delegate) {
	_playbackCompleteDelegate = delegate;
}

void TripdeckMediaManager::_playVideoInternal() {
	if (_videoPlayer) {
		_videoPlayer->play();
	}
}

void TripdeckMediaManager::_playLedInternal() {
	if (_ledPlayer) {
		_ledPlayer->play();
	}
}

void TripdeckMediaManager::_stopVideoInternal() {
	if (_videoPlayer) {
		_videoPlayer->stop();
	}
}

void TripdeckMediaManager::_stopLedInternal() {
	if (_ledPlayer) {
		_ledPlayer->stop();
	}
}

void TripdeckMediaManager::_pauseVideoInternal() {
	if (_videoPlayer) {
		_videoPlayer->pause();
	}
}

void TripdeckMediaManager::_pauseLedInternal() {
	if (_ledPlayer) {
		_ledPlayer->pause();
	}
}

void TripdeckMediaManager::_ledPlayerPlaybackComplete(const MediaPlayer::PlaybackCompleteArgs& args) {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Received led player playback complete call with current media set to :" << args.currentMedia << std::endl;
	#endif
	
	if (_playbackCompleteDelegate) {
		TripdeckStateChangedArgs stateArgs = { _currentState, 0, args.currentMedia, Led, args.playbackOption };
		_playbackCompleteDelegate->execute((CommandArgs)&stateArgs);
	}
}

void TripdeckMediaManager::_videoPlayerPlaybackComplete(const MediaPlayer::PlaybackCompleteArgs& args) {
	#if ENABLE_MEDIA_DEBUG
	// TODO: Remove debug code
	std::cout << "Received video player playback complete call with current media set to :" << args.currentMedia << std::endl;
	#endif

	if (_playbackCompleteDelegate) {
		TripdeckStateChangedArgs stateArgs = { _currentState, args.currentMedia, 0, Video, args.playbackOption };
		_playbackCompleteDelegate->execute((CommandArgs)&stateArgs);
	}
}
