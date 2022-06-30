#ifndef _TRIPDECK_BEHAVIOR_LEADER_H_
#define _TRIPDECK_BEHAVIOR_LEADER_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "Tripdeck.h"

class TripdeckLeader : public Tripdeck {
	public:
		TripdeckLeader(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		~TripdeckLeader();
		void init() override;
		void run() override;
	private:
		TripdeckMediaOption _nextMediaActionOption;
		MediaPlayer::MediaPlayerState _nextMediaPlayerState;
		std::unordered_map<char, TripdeckStatus> _nodeIdToStatus;
		std::vector<pair<int64_t, void (TripdeckLeader::*)(void)>> _oneShotActions;
		std::vector<Input*> _buttons;
		bool _followersSynced = false;

		void _onStateChanged();
		void _updateFollowers(TripdeckStateChangedArgs& args);
		void _handleSerialInput(InputArgs& args) override;
		void _handleDigitalInput(InputArgs& data);
		void _updateFollowerState(char id, TripdeckStateChangedArgs& args);
		void _addOneShotAction(void (TripdeckLeader::*action)(void), int64_t wait);
		void _runOneShotAction();
		void _setMediaNotificationAction(TripdeckMediaOption option, MediaPlayer::MediaPlayerState state);
		void _mediaNotificationAction();
		bool _verifySynced();
		void _runStartup();
		void _executeReveal();
		void _handleChainPull(char id);
		void _handleReset();

		class DigitalInputDelegate : public Command {
			public:
				DigitalInputDelegate(TripdeckLeader* owner);
				~DigitalInputDelegate();
				void execute(CommandArgs args) override;
			private:
				TripdeckLeader* _owner;
		};
};

#endif