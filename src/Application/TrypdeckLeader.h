#ifndef _TRIPDECK_BEHAVIOR_LEADER_H_
#define _TRIPDECK_BEHAVIOR_LEADER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "Trypdeck.h"

class TrypdeckLeader : public Trypdeck {
	public:
		TrypdeckLeader(TrypdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		~TrypdeckLeader();
		void init() override;
		void run() override;
	private:
		TrypdeckMediaOption _nextMediaActionOption;
		MediaPlayer::MediaPlayerState _nextMediaPlayerState;
		std::unordered_map<char, TrypdeckStatus> _nodeIdToStatus;
		std::vector<pair<int64_t, void (TrypdeckLeader::*)(void)>> _oneShotActions;
		std::vector<Input*> _buttons;
		int64_t _lastPlaybackCompleteMessageMillis = 0;
		bool _followersSynced = false;
		bool _revealTriggered = false;
		bool _chainPulled = false;
		bool _firstPull = true;

		// run
		void _runStartup();
		bool _verifySynced();
		bool _verifyAllPulled();
		void _runOneShotActions();
		// state and input
		void _onStateChanged();
		void _handleSerialInput(const InputArgs& args) override;
		void _receiveStartupNotification(char id, const std::string& buffer);
		void _receiveFollowerStatusUpdate(char id, const std::string& buffer);
		void _handleMediaPlaybackCompleteMessage(char id, const std::string& buffer);
		void _updateStateFollower(char id, TrypdeckStateChangedArgs& args);
		void _updateStateFollowers(TrypdeckStateChangedArgs& args);
		void _updateMediaStateFollower(char id, TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state);
		void _updateMediaStateUniversal(TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state);
		void _triggerLedAnimationForState(TrypdeckState state, MediaPlayer::MediaPlaybackOption playbackOption = MediaPlayer::OneShot);
		void _handleDigitalInput(const InputArgs& args);
		void _handleChainPull(char id);
		void _handleReset();
		void _handleShutdown();
		// events
		void _handleMediaPlayerPlaybackComplete(const TrypdeckStateChangedArgs& args) override;
		// actions
		void _addOneShotAction(void (TrypdeckLeader::*action)(void), int64_t wait);
		void _setMediaUpdateUniversalAction(TrypdeckMediaOption option, MediaPlayer::MediaPlayerState state, int64_t wait = 5);
		void _updateMediaStateUniversalAction();
		void _cancelOneShotActions();
		void _executePreReveal();
		void _executeReveal();
		void _returnToWait();
		
};

#endif