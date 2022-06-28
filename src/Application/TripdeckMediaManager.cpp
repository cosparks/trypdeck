#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "Clock.h"
#include "TripdeckMediaManager.h"

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
}

TripdeckMediaManager::~TripdeckMediaManager() { }

void TripdeckMediaManager::init() {
	// initialize all runnable objects
	for (Runnable* runnable : _runnableObjects) {
		runnable->init();
	}

	// hook up media players to data manager
	if (_ledPlayer)
		_dataManager->addMediaListener(_ledPlayer);
	if (_videoPlayer)
		_dataManager->addMediaListener(_videoPlayer);
}

void TripdeckMediaManager::run() {
	for (Runnable* runnable : _runnableObjects) {
			runnable->run();
	}
}

void TripdeckMediaManager::play(TripdeckMediaPlaybackOption option) {
	switch (option) {
		case Video:
			if (_videoPlayer)
				_videoPlayer->play();
			break;
		case Led:
			if (_ledPlayer)
				_ledPlayer->play();
			break;
		default:
			if (_videoPlayer)
				_videoPlayer->play();
			if (_ledPlayer)
				_ledPlayer->play();
			break;
	} 
}

void TripdeckMediaManager::stop(TripdeckMediaPlaybackOption option) {
	switch (option) {
		case Video:
			if (_videoPlayer)
				_videoPlayer->stop();
			break;
		case Led:
			if (_ledPlayer)
				_ledPlayer->stop();
			break;
		default:
			if (_videoPlayer)
				_videoPlayer->stop();
			if (_ledPlayer)
				_ledPlayer->stop();
			break;
	}
}

void TripdeckMediaManager::pause(TripdeckMediaPlaybackOption option) {
	switch (option) {
		case Video:
			if (_videoPlayer)
				_videoPlayer->pause();
			break;
		case Led:
			if (_ledPlayer)
				_ledPlayer->pause();
			break;
		default:
			if (_videoPlayer)
				_videoPlayer->pause();
			if (_ledPlayer)
				_ledPlayer->pause();
			break;
	} 
}

void TripdeckMediaManager::updateState(TripdeckStateChangedArgs& args) {
	_currentState = args.newState;

	if (_videoPlayer) {
		uint32_t videoId = 0;
		if (args.videoId == 0) {
			srand((uint32_t)Clock::instance().millis());
			videoId = getRandomVideoId(_currentState);
		} else {
			videoId = args.videoId;
		}

		_videoPlayer->setCurrentMedia(videoId, args.loop ? MediaPlayer::MediaPlaybackOption::Loop : MediaPlayer::MediaPlaybackOption::OneShot);

		if (!args.syncVideo)
			_videoPlayer->play();
	}

	if (_ledPlayer) {
		uint32_t ledId = 0;
		if (args.ledId == 0) {
			srand((uint32_t)Clock::instance().millis());
			ledId = getRandomVideoId(_currentState);
		} else {
			ledId = args.ledId;
		}

		_ledPlayer->setCurrentMedia(ledId, args.loop ? MediaPlayer::MediaPlaybackOption::Loop : MediaPlayer::MediaPlaybackOption::OneShot);

		if (!args.syncLeds)
			_ledPlayer->play();
	}
}

void TripdeckMediaManager::addVideoFolder(TripdeckState state, const char* folder) {
	_videoPlayer->addMediaFolder(folder);
	_stateToVideoFolder[state] = folder;
}

void TripdeckMediaManager::addLedFolder(TripdeckState state, const char* folder) {
	_ledPlayer->addMediaFolder(folder);
	_stateToLedFolder[state] = folder;
}

uint32_t TripdeckMediaManager::getRandomVideoId(TripdeckState state) {
	const auto& videoFiles = _dataManager->getFileIdsFromFolder(_stateToVideoFolder[state]);

	if (videoFiles.size() == 0) {
		std::string message = "Error: video folder for current state (" + std::to_string(_currentState) + ") does not contain any files";
		throw std::runtime_error(message);
	}

	return videoFiles[rand() % videoFiles.size()];
}

uint32_t TripdeckMediaManager::getRandomLedId(TripdeckState state) {
	const auto& ledFiles = _dataManager->getFileIdsFromFolder(_stateToLedFolder[_currentState]);

	if (ledFiles.size() == 0) {
		std::string message = "Error: led folder for current state (" + std::to_string(_currentState) + ") does not contain any files";
		throw std::runtime_error(message);
	}

	return ledFiles[rand() % ledFiles.size()];
}