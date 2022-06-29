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
		int64_t _nextActionMillis = 0;

		void _onStateChanged(TripdeckStateChangedArgs& args);
		void _runTimedAction(void (TripdeckFollower::*action)(void), int64_t interval = NOTIFICATION_INTERVAL);
		void _sendConnectingMessage();
		void _sendStatusUpdate();
		void _handleSerialInput(InputArgs& args) override;
		bool _parseStateChangedMessage(const std::string& buffer, TripdeckStateChangedArgs& args);
};

#endif