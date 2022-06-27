#ifndef _TRIPDECK_BEHAVIOR_H_
#define _TRIPDECK_BEHAVIOR_H_

#include <vector>

#include "settings.h"
#include "td_util.h"
#include "Runnable.h"
#include "TripdeckMediaManager.h"
#include "InputManager.h"
#include "InputThreadedSerial.h"

using namespace td_util;

// length of any and all headers used for communication on the Serial network
#define HEADER_LENGTH 3
// startup notification structure: "sn/ID"
// ID is unqiue identifier for sender
#define STARTUP_NOTIFICATION_HEADER "sn/"
// startup notification structure: "sc/ID/STATE(/VIDEOHASH/LEDHASH)"
// STATE is TripdeckState to change to and (/VIDEOHASH/LEDHASH) are optional arguments for video and led file IDs
// ID is unique identifier for intended recipient
#define STATE_CHANGED_HEADER "sc/"

/**
 * @brief Abstract class the children of which will encapsulate the unique behavior for leader and follower
 * @note essentially just manages application state and leader/follower networking behavior
 */
class Tripdeck : public Runnable {
	public:
		Tripdeck(TripdeckMediaManager* mediaManager, InputManager* inputManager, Serial* serial);
		virtual ~Tripdeck() { }
		void init() override;
		void run() override;
		void setStateChangedDelegate(Command* delegate);
		virtual void handleMediaChanged(TripdeckStateChangedArgs& args) = 0;

	protected:
		class SerialInputDelegate : public Command {
			public:
				SerialInputDelegate(Tripdeck* owner);
				~SerialInputDelegate();
				void execute(CommandArgs args) override;
			private:
				Tripdeck* _owner;
		};
		
		TripdeckMediaManager* _mediaManager = NULL;
		InputManager* _inputManager = NULL;
		Serial* _serial = NULL;
		TripdeckState _currentState;
		Command* _stateChangedDelegate = NULL;
		InputThreadedSerial* _serialInput = NULL;
		SerialInputDelegate* _serialInputDelegate = NULL;

		virtual void _onStateChanged(TripdeckStateChangedArgs& args) = 0;
		virtual void _handleSerialInput(InputArgs& args) = 0;
};

#endif