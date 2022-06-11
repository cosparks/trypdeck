#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"

#include "Runnable.h"
#include "DataManager.h"
#include "MediaPlayer.h"
#include "KeyboardInput.h"

class Tripdeck : public Runnable {
	public:
		enum TripdeckState { Startup, Wait, Pulled, Reveal };
		Tripdeck(DataManager* dataManager, MediaPlayer* _ledPlayer, MediaPlayer* _videoPlayer);
		~Tripdeck();
		void init();
		void run();
		void addVideoFolder(TripdeckState state, const char* folder);
		void addLedFolder(TripdeckState state, const char* folder);
		void handleKeyboardInput(char input);

	private:
		DataManager* _dataManager;
		// needs a more definitive type of input handler
		KeyboardInput<Tripdeck>* _keyboardInputHandler;
		MediaPlayer* _ledPlayer;
		MediaPlayer* _videoPlayer;
		std::unordered_map<TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		TripdeckState _state;
		bool _run;

		void _onStateChanged();
		void _handleStartup();
		void _handleWait();
		void _handlePulled();
		void _handleReveal();
};

#endif