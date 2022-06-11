#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "Clock.h"
#include "Tripdeck.h"

Tripdeck::Tripdeck(DataManager* dataManager, MediaPlayer* ledPlayer, MediaPlayer* videoPlayer) :
	_dataManager(dataManager), _ledPlayer(ledPlayer), _videoPlayer(videoPlayer) {
		if (!_dataManager)
			throw std::runtime_error("Error: Tripdeck::_dataManager cannot be NULL");

		_runnableObjects.push_back(_ledPlayer);

		if (_ledPlayer) 
			_runnableObjects.push_back(_ledPlayer);
		if (_videoPlayer)
			_runnableObjects.push_back(_videoPlayer);
	}

Tripdeck::~Tripdeck() { }

void Tripdeck::init() {
	for (Runnable* runnable : _runnableObjects) {
		runnable->init();
	}

	if (_ledPlayer)
		_dataManager->addMediaListener(_ledPlayer);
	if (_videoPlayer)
		_dataManager->addMediaListener(_videoPlayer);

	_state = Startup;
	_run = true;
	_onStateChanged();
}

void Tripdeck::run() {
	while (_run) {
		// check inputs
		for (Runnable* runnable : _runnableObjects) {
			runnable->run();
		}
	}
}

void Tripdeck::addVideoFolder(TripdeckState state, const char* folder) {
	_videoPlayer->addMediaFolder(folder);
	_stateToVideoFolder[state] = folder;
}

void Tripdeck::addLedFolder(TripdeckState state, const char* folder) {
	_ledPlayer->addMediaFolder(folder);
	_stateToLedFolder[state] = folder;
}

void Tripdeck::handleKeyboardInput(char input) {
	std::cout << "Key pressed with char val " << +input << std::endl;
}

void Tripdeck::_onStateChanged() {
	srand((uint32_t)Clock::instance().millis());
	
	// set random led file from folder
	const auto& ledFiles = _dataManager->getFileIdsFromFolder(_stateToLedFolder[_state]);
	uint32_t randomLedFile = ledFiles[rand() % ledFiles.size()];
	_ledPlayer->setCurrentMedia(randomLedFile, MediaPlayer::MediaPlaybackOption::Loop);
	_ledPlayer->play();

	// set random video file from folder
	const auto& videoFiles = _dataManager->getFileIdsFromFolder(_stateToVideoFolder[_state]);
	uint32_t randomVideoFile = videoFiles[rand() % videoFiles.size()];	
	_videoPlayer->setCurrentMedia(randomVideoFile, MediaPlayer::MediaPlaybackOption::Loop);
	_ledPlayer->play();
}