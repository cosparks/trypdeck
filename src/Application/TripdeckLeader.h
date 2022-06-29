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
		void handleMediaChanged(TripdeckStateChangedArgs& args) override;
	private:
		std::unordered_map<std::string, TripdeckStatus> _nodeIdToStatus;
		bool _followersSynced = false;

		void _onStateChanged();
		void _updateFollowers(TripdeckStateChangedArgs& args);
		void _handleSerialInput(InputArgs& args) override;
		void _handleUserInput(InputArgs* data);
		void _updateFollowerState(const std::string& id, TripdeckStateChangedArgs& args);
		bool _verifySynced();
		void _runStartup();
};

#endif