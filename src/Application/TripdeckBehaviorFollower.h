#ifndef _TRIPDECK_BEHAVIOR_FOLLOWER_H_
#define _TRIPDECK_BEHAVIOR_FOLLOWER_H_

#include "TripdeckBehavior.h"

class TripdeckBehaviorFollower : public TripdeckBehavior {
	public:
		TripdeckBehaviorFollower(InputManager* inputManager, Serial* serial);
		~TripdeckBehaviorFollower();
		void init() override;
		void run() override;
	private:
		int64_t _nextActionMillis = 0;

		void _onStateChanged(TripdeckStateChangedArgs& args) override;
		void _notifyLeader();
		void _handleSerialInput(const std::string& buffer) override;
};

#endif