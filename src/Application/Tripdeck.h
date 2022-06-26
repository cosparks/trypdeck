#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "DataManager.h"
#include "InputManager.h"
#include "TripdeckBehavior.h"
#include "MediaPlayer.h"

using namespace td_util;

class Tripdeck : public Runnable {
	public:
		Tripdeck(DataManager* dataManager, TripdeckBehavior* behavior, MediaPlayer* _videoPlayer, MediaPlayer* _ledPlayer = NULL);
		~Tripdeck();
		void init();
		void run();
		void addVideoFolder(TripdeckBehavior::TripdeckState state, const char* folder);
		void addLedFolder(TripdeckBehavior::TripdeckState state, const char* folder);
		void handleKeyboardInput(int32_t input);

	private:
		DataManager* _dataManager = NULL;
		TripdeckBehavior* _behavior = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		std::unordered_map<TripdeckBehavior::TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TripdeckBehavior::TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		bool _run;

		void _stateChanged(TripdeckBehavior::TripdeckStateChangedArgs* args);
		void _onStateChanged();

		class TripdeckStateChangedDelegate : public Command {
			public:
				TripdeckStateChangedDelegate(Tripdeck* owner);
				~TripdeckStateChangedDelegate();
				void execute(CommandArgs args) override;
			private:
				Tripdeck* _owner = NULL;
		};
};

#endif