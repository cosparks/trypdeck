#ifndef _TRIPDECK_BEHAVIOR_FOLLOWER_H_
#define _TRIPDECK_BEHAVIOR_FOLLOWER_H_

#define NOTIFICATION_INTERVAL 2000

#include "Trypdeck.h"

class TrypdeckFollower : public Trypdeck {
	public:
		TrypdeckFollower(TrypdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		~TrypdeckFollower();
		void init() override;
		void run() override;

	private:

		// state and networking
		void _onStateChanged(TrypdeckStateChangedArgs& args);
		void _pingLeader();
		void _sendStatusUpdate();
		void _handleSerialInput(const InputArgs& args) override;
		void _handleStateChangedMessage(const std::string& buffer);
		void _handlePlayMediaFromArgsMessage(const std::string& buffer);
		bool _parseStateChangedMessage(const std::string& buffer, TrypdeckStateChangedArgs& args);
		// events
		void _handleMediaPlayerPlaybackComplete(const TrypdeckStateChangedArgs& args) override;

};

#endif