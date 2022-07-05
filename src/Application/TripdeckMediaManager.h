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
		void updateState(const TripdeckStateChangedArgs& args);
		void updateStateVideo(const TripdeckStateChangedArgs& args);
		void updateStateLed(const TripdeckStateChangedArgs& args);
		void play(TripdeckMediaOption option = Both);
		void stop(TripdeckMediaOption option = Both);
		void pause(TripdeckMediaOption option = Both);
		void addVideoFolder(TripdeckState state, const char* folder);
		void addLedFolder(TripdeckState state, const char* folder);
		uint32_t getRandomVideoId(TripdeckState state);
		uint32_t getRandomLedId(TripdeckState state);
		void setPlaybackCompleteDelegate(Command* delegate);

	private:
		TripdeckState _currentState = Unknown;
		DataManager* _dataManager = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		Command* _playbackCompleteDelegate;
		std::unordered_map<TripdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TripdeckState, std::string> _stateToLedFolder;
		std::vector<Runnable*> _runnableObjects;
		bool _run;

		void _playVideoInternal();
		void _playLedInternal();
		void _stopVideoInternal();
		void _stopLedInternal();
		void _pauseVideoInternal();
		void _pauseLedInternal();
		void _ledPlayerPlaybackComplete(const MediaPlayer::PlaybackCompleteArgs& args);
		void _videoPlayerPlaybackComplete(const MediaPlayer::PlaybackCompleteArgs& args);
};

#endif