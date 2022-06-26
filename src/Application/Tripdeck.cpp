#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include "Clock.h"
#include "Tripdeck.h"

#define RUN_TIME_MILLIS 330000000
#define VIDEO_PLAY_INTERVAL 10000
#define VLC_DELAY 600

Tripdeck::Tripdeck(DataManager* dataManager, InputManager* inputManager, MediaPlayer* videoPlayer, MediaPlayer* ledPlayer) :
	_dataManager(dataManager), _inputManager(inputManager), _videoPlayer(videoPlayer), _ledPlayer(ledPlayer) {
		if (!_dataManager)
			throw std::runtime_error("Error: Tripdeck::_dataManager cannot be NULL");

		_runnableObjects.push_back(_dataManager);
		_runnableObjects.push_back(_inputManager);

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

void Tripdeck::addInputs(const std::vector<Input*>& inputs) {
	TripdeckDelegate* delegate = new TripdeckDelegate(this);
	for (Input* input : inputs) {
		_inputManager.addInput();
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

void Tripdeck::_handleInput(InputData* data) {
	
}

void Tripdeck::handleKeyboardInput(int32_t input) {
	std::cout << "Key pressed with char val " << input << std::endl;
}

void Tripdeck::_onStateChanged() {

	switch (_state) {
		case Startup:
			_onStateChangedStartup();
			break;
		case Wait:
			_onStateChangedWait();
			break;
		case Pulled:
			_onStateChangedPulled();
			break;
		case Reveal:
			_onStateChangedReveal();
			break;
		default:
			// do nothing
			break;
	}

	srand((uint32_t)Clock::instance().millis());
}

#ifdef Leader
void _onStateChangedStartup() {

}

void _onStateChangedWait() {

}

void _onStateChangedPulled() {

}

void _onStateChangedReveal() {

}
#endif

#ifdef Follower
void Tripdeck::_onStateChangedStartup() {
	if (_ledPlayer) {
		// set random led file from folder
		const auto& ledFiles = _dataManager->getFileIdsFromFolder(_stateToLedFolder[_state]);
		uint32_t randomLedFile = ledFiles[rand() % ledFiles.size()];
		_ledPlayer->setCurrentMedia(randomLedFile, MediaPlayer::MediaPlaybackOption::Loop);
		_ledPlayer->play();
	}

	if (_videoPlayer) {
		// set random video file from folder
		const auto& videoFiles = _dataManager->getFileIdsFromFolder(_stateToVideoFolder[_state]);
		uint32_t randomVideoFile = videoFiles[rand() % videoFiles.size()];
		_videoPlayer->setCurrentMedia(randomVideoFile, MediaPlayer::MediaPlaybackOption::Loop);
		_videoPlayer->play();
	}
}

void _onStateChangedWait() {

}

void _onStateChangedPulled() {

}

void _onStateChangedReveal() {

}
#endif

Tripdeck::TripdeckDelegate::TripdeckDelegate(Tripdeck* owner) {
	_owner = owner;
}

Tripdeck::TripdeckDelegate::~TripdeckDelegate() { }

void Tripdeck::TripdeckDelegate::execute(CommandArgs args) {
	_owner->_handleInput((InputData*)args);
}