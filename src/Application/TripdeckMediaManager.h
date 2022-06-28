#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "DataManager.h"
#include "InputManager.h"
#include "TripdeckState.h"
#include "MediaPlayer.h"

using namespace td_util;

class TripdeckMediaManager : public Runnable {
	public:
		TripdeckMediaManager(DataManager* dataManager, MediaPlayer* _videoPlayer, MediaPlayer* _ledPlayer = NULL);
		~TripdeckMediaManager();
		void init() override;
		void run() override;
		void updateState(TripdeckStateChangedArgs& args);
		void addVideoFolder(TripdeckState state, const char* folder);
		void addLedFolder(TripdeckState state, const char* folder);
		uint32_t getRandomVideoId(TripdeckState state);
		uint32_t getRandomLedId(TripdeckState state);

	private:
		TripdeckState _currentState = Inactive;
		DataManager* _dataManager = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		std::unordered_map<TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		bool _run;

		void _stateChanged(TripdeckStateChangedArgs* args);
		void _onStateChanged();

		// class TripdeckStateChangedDelegate : public Command {
		// 	public:
		// 		TripdeckStateChangedDelegate(TripdeckMediaManager* owner);
		// 		~TripdeckStateChangedDelegate();
		// 		void execute(CommandArgs args) override;
		// 	private:
		// 		TripdeckMediaManager* _owner = NULL;
		// };
};

#endif