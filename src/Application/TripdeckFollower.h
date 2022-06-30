#ifndef _TRIPDECK_BEHAVIOR_FOLLOWER_H_
#define _TRIPDECK_BEHAVIOR_FOLLOWER_H_

#define NOTIFICATION_INTERVAL 2000

#include "Tripdeck.h"

class TripdeckFollower : public Tripdeck {
	public:
		TripdeckFollower(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		~TripdeckFollower();
		void init() override;
		void run() override;

	private:

		void _onStateChanged(TripdeckStateChangedArgs& args);
		void _pingLeader();
		void _sendStatusUpdate();
		void _handleSerialInput(InputArgs& args) override;
		bool _parseStateChangedMessage(const std::string& buffer, TripdeckStateChangedArgs& args);
		void _populateStateArgsFromBuffer(const std::string& buffer, TripdeckStateChangedArgs& args);
};

#endif