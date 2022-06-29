#ifndef _TRIPDECK_BEHAVIOR_LEADER_H_
#define _TRIPDECK_BEHAVIOR_LEADER_H_

#include <string>
#include <unordered_map>

#include "Tripdeck.h"

class TripdeckLeader : public Tripdeck {
	public:
		TripdeckLeader(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		~TripdeckLeader();
		void init() override;
		void run() override;
	private:
		std::unordered_map<std::string, TripdeckStatus> _nodeIdToStatus;
		void (TripdeckLeader::*_oneShotAction)(void)  = NULL;
		int64_t _nextOneShotActionMillis = 0;
		TripdeckMediaOption _nextMediaActionOption;
		MediaPlayer::MediaPlayerState _nextMediaPlayerState;
		bool _followersSynced = false;

		void _onStateChanged();
		void _updateFollowers(TripdeckStateChangedArgs& args);
		void _handleSerialInput(InputArgs& args) override;
		void _handleUserInput(InputArgs* data);
		void _updateFollowerState(const std::string& id, TripdeckStateChangedArgs& args);
		void _setOneShotAction(void (TripdeckLeader::*action)(void), int64_t wait);
		void _runOneShotAction();
		void _setMediaNotificationAction(TripdeckMediaOption option, MediaPlayer::MediaPlayerState state);
		void _mediaNotificationAction();
		bool _verifySynced();
		void _runStartup();
};

#endif