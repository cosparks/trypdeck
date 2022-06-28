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

void TripdeckMediaManager::updateState(TripdeckStateChangedArgs& args) {
	_currentState = args.newState;

	std::cout << "State changed args received" << std::endl;
	std::cout << "New state == " << args.newState << std::endl;
	std::cout << "Synchronize video: " << (args.syncVideo ? "True" : "False") << std::endl;
	std::cout << "Synchronize led: " << (args.syncLeds ? "True" : "False") << std::endl;
	std::cout << "Video hash: " << args.videoId << std::endl;
	std::cout << "Led hash: " << args.ledId << std::endl;

	// TODO: specify state behavior based on state changed args
	_onStateChanged();
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

void TripdeckMediaManager::_stateChanged(TripdeckStateChangedArgs* args) {

}

void TripdeckMediaManager::_onStateChanged() {
	srand((uint32_t)Clock::instance().millis());
	
	if (_videoPlayer) {
		uint32_t randomVideoFile = getRandomVideoId(_currentState);
		_videoPlayer->setCurrentMedia(randomVideoFile, MediaPlayer::MediaPlaybackOption::Loop);
		_videoPlayer->play();
	}

	if (_ledPlayer) {
		uint32_t randomLedFile = getRandomLedId(_currentState);
		_ledPlayer->setCurrentMedia(randomLedFile, MediaPlayer::MediaPlaybackOption::Loop);
		_ledPlayer->play();
	}
}

// TripdeckMediaManager::TripdeckStateChangedDelegate::TripdeckStateChangedDelegate(TripdeckMediaManager* owner) {
// 	_owner = owner;
// }

// TripdeckMediaManager::TripdeckStateChangedDelegate::~TripdeckStateChangedDelegate() { }

// void TripdeckMediaManager::TripdeckStateChangedDelegate::execute(CommandArgs args) {
// 	_owner->_stateChanged((TripdeckStateChangedArgs*)args);
// }