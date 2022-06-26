#ifndef _TRIPDECK_BEHAVIOR_H_
#define _TRIPDECK_BEHAVIOR_H_

#include <vector>

#include "settings.h"
#include "td_util.h"
#include "Runnable.h"
#include "InputManager.h"
#include "Serial.h"

using namespace td_util;

#define HEADER_LENGTH 3
// startup notification structure: "sn/ID"
// ID is unqiue identifier for another pi on network
#define STARTUP_NOTIFICATION_HEADER "sn/"
// startup notification structure: "sc/ID/STATE(/VIDEOHASH/LEDHASH)"
// STATE is TripdeckState to change to and (/VIDEOHASH/LEDHASH) are optional arguments for video and led file IDs
#define STATE_CHANGED_HEADER "sc/"

// LEADER MUST:
//		on startup, listen for startup messages from followers and establish unique IDs for each one
//		listen for digital input from pins
//		send out serial messages to followers when application state changes
// FOLLOWER MUST:
//		on startup, repeatedly send message to leader with unique identifier
//		listen for serial messages
//		change states when message is received

/**
 * @brief Abstract class the children of which will encapsulate the unique behavior for leader and follower
 * @note essentially just manages application state and leader/follower networking behavior
 */
class TripdeckBehavior : public Runnable {
	public:
		enum TripdeckState { Startup, Wait, Pulled, Reveal };
		struct TripdeckStateChangedArgs {
			TripdeckState newState;
			uint32_t videoId;
			uint32_t ledId;
			bool syncVideo;
			bool syncLeds;
		};

		TripdeckBehavior(InputManager* inputManager, Serial* serial);
		virtual ~TripdeckBehavior() { }
		void init() override;
		void run() override;
		TripdeckState getState();
		void setStateChangedDelegate(Command* delegate);
	protected:
		TripdeckState _currentState;
		InputManager* _inputManager = NULL;
		Serial* _serial = NULL;
		Command* _stateChangedDelegate = NULL;

		virtual void _onStateChanged(TripdeckStateChangedArgs& args) = 0;
		virtual void _handleSerialInput(const std::string& buffer) = 0;

		class SerialInputDelegate : public Command {
			public:
				SerialInputDelegate(TripdeckBehavior* owner);
				~SerialInputDelegate();
				void execute(CommandArgs args) override;
			private:
				TripdeckBehavior* _owner;
		};
};

#endif