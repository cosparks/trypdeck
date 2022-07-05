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

		// state and networking
		void _onStateChanged(TripdeckStateChangedArgs& args);
		void _pingLeader();
		void _sendStatusUpdate();
		void _handleSerialInput(const InputArgs& args) override;
		void _handleStateChangedMessage(const std::string& buffer);
		void _handlePlayMediaFromArgsMessage(const std::string& buffer);
		bool _parseStateChangedMessage(const std::string& buffer, TripdeckStateChangedArgs& args);
		// events
		void _handleMediaPlayerPlaybackComplete(const TripdeckStateChangedArgs& args) override;

};

#endif