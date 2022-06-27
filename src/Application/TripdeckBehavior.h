#ifndef _TRIPDECK_BEHAVIOR_H_
#define _TRIPDECK_BEHAVIOR_H_

#include <vector>

#include "settings.h"
#include "td_util.h"
#include "Runnable.h"
#include "InputManager.h"
#include "InputThreadedSerial.h"

using namespace td_util;

// lendth of any and all headers used for communication on the Serial network
#define HEADER_LENGTH 3
// startup notification structure: "sn/ID"
// ID is unqiue identifier for sender
#define STARTUP_NOTIFICATION_HEADER "sn/"
// startup notification structure: "sc/ID/STATE(/VIDEOHASH/LEDHASH)"
// STATE is TripdeckState to change to and (/VIDEOHASH/LEDHASH) are optional arguments for video and led file IDs
// ID is unique identifier for intended recipient
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
		class SerialInputDelegate : public Command {
			public:
				SerialInputDelegate(TripdeckBehavior* owner);
				~SerialInputDelegate();
				void execute(CommandArgs args) override;
			private:
				TripdeckBehavior* _owner;
		};
		
		TripdeckState _currentState;
		InputManager* _inputManager = NULL;
		Serial* _serial = NULL;
		Command* _stateChangedDelegate = NULL;
		InputThreadedSerial* _serialInput = NULL;
		SerialInputDelegate* _serialInputDelegate = NULL;

		virtual void _onStateChanged(TripdeckStateChangedArgs& args) = 0;
		virtual void _handleSerialInput(InputArgs& args) = 0;
};

#endif