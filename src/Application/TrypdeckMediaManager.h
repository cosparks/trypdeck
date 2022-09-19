#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "DataManager.h"
#include "InputManager.h"
#include "TrypdeckState.h"
#include "MediaPlayer.h"

using namespace td_util;

class TrypdeckMediaManager : public Runnable {
	public:
		TrypdeckMediaManager(DataManager* dataManager, MediaPlayer* _videoPlayer, MediaPlayer* _ledPlayer = NULL);
		~TrypdeckMediaManager();
		void init() override;
		void run() override;
		void updateState(const TrypdeckStateChangedArgs& args);
		void updateStateVideo(const TrypdeckStateChangedArgs& args);
		void updateStateLed(const TrypdeckStateChangedArgs& args);
		void play(TrypdeckMediaOption option = Both);
		void stop(TrypdeckMediaOption option = Both);
		void pause(TrypdeckMediaOption option = Both);
		void addVideoFolder(TrypdeckState state, const char* folder);
		void addLedFolder(TrypdeckState state, const char* folder);
		uint32_t getRandomVideoId(TrypdeckState state);
		uint32_t getRandomLedId(TrypdeckState state);
		void setPlaybackCompleteDelegate(Command* delegate);

	private:
		TrypdeckState _currentState = Unknown;
		DataManager* _dataManager = NULL;
		MediaPlayer* _videoPlayer = NULL;
		MediaPlayer* _ledPlayer = NULL;
		Command* _playbackCompleteDelegate;
		MediaListener* _mediaListener = NULL;
		std::unordered_map<TrypdeckState, std::string> _stateToVideoFolder;
		std::unordered_map<TrypdeckState, std::string> _stateToLedFolder;
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