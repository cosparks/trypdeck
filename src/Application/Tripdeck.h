#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "DataManager.h"
#include "InputManager.h"
#include "MediaPlayer.h"

using namespace td_util;

class Tripdeck : public Runnable {
	public:
		enum TripdeckState { Startup, Wait, Pulled, Reveal };
		Tripdeck(DataManager* dataManager, InputManager* inputManager, MediaPlayer* _videoPlayer, MediaPlayer* _ledPlayer = NULL);
		~Tripdeck();
		void init();
		void run();
		void addInputs(const std::vector<Input*>& inputs);
		void addVideoFolder(TripdeckState state, const char* folder);
		void addLedFolder(TripdeckState state, const char* folder);
		void handleKeyboardInput(int32_t input);

	private:
		DataManager* _dataManager = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		InputManager* _inputManager = NULL;
		std::unordered_map<TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		TripdeckState _state;
		bool _run;

		void _handleInput(InputData* id);
		void _onStateChanged();
		void _onStateChangedStartup();
		void _onStateChangedWait();
		void _onStateChangedPulled();
		void _onStateChangedReveal();

		class TripdeckDelegate : public Command {
			public:
				TripdeckDelegate(Tripdeck* owner);
				~TripdeckDelegate();
				void execute(CommandArgs args) override;
			private:
				Tripdeck* _owner = NULL;
		};
};

#endif