#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "DataManager.h"
#include "InputManager.h"
#include "Tripdeck.h"
#include "MediaPlayer.h"

using namespace td_util;

class TripdeckMedia : public Runnable {
	public:
		TripdeckMedia(DataManager* dataManager, Tripdeck* behavior, MediaPlayer* _videoPlayer, MediaPlayer* _ledPlayer = NULL);
		~TripdeckMedia();
		void init();
		void run();
		void addVideoFolder(Tripdeck::TripdeckState state, const char* folder);
		void addLedFolder(Tripdeck::TripdeckState state, const char* folder);
		void handleKeyboardInput(int32_t input);

	private:
		DataManager* _dataManager = NULL;
		Tripdeck* _behavior = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		std::unordered_map<Tripdeck::TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<Tripdeck::TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		bool _run;

		void _stateChanged(Tripdeck::TripdeckStateChangedArgs* args);
		void _onStateChanged();

		class TripdeckStateChangedDelegate : public Command {
			public:
				TripdeckStateChangedDelegate(TripdeckMedia* owner);
				~TripdeckStateChangedDelegate();
				void execute(CommandArgs args) override;
			private:
				TripdeckMedia* _owner = NULL;
		};
};

#endif