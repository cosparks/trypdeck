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
// if VIDEOHASH or LEDHASH are zero, follower will select and play random media associated with that state
// ID is unique identifier for intended recipient
#define STATE_CHANGED_HEADER "sc/"
// follower synced message header: fs/ID/STATE
// sent during Follower's connected phase
#define STATUS_UPDATE_HEADER "su/"
// message dictating that follower play media: "jm/ID/OPTION"
// OPTION is TripdeckMediaOption
#define PLAY_MEDIA_HEADER "jm/"
// message dictating that follower stop media: "sm/ID/OPTION"
// OPTION is TripdeckMediaOption
#define STOP_MEDIA_HEADER "sm/"
// message dictating that follower pause media: "pm/ID/OPTION"
// OPTION is TripdeckMediaOption
#define PAUSE_MEDIA_HEADER "pm/"

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

		struct MediaHashes {
			uint32_t videoHash;
			uint32_t ledHash;
		};
		
		TripdeckMediaManager* _mediaManager = NULL;
		InputManager* _inputManager = NULL;
		Serial* _serial = NULL;
		TripdeckState _currentState;
		Command* _stateChangedDelegate = NULL;
		InputThreadedSerial* _serialInput = NULL;
		SerialInputDelegate* _serialInputDelegate = NULL;
		TripdeckStatus _status;
		bool _run;

		virtual void _handleSerialInput(InputArgs& args) = 0;
		MediaHashes _parseMediaHashes(const std::string& buffer);
		const std::string _hashToHexString(uint32_t hash);

		inline const std::string _parseHeader(const std::string& buffer) {
			return buffer.substr(0, HEADER_LENGTH);
		}

		inline const std::string _parseId(const std::string& buffer) {
			return buffer.substr(HEADER_LENGTH, 1);
		}

		inline TripdeckState _parseState(const std::string& buffer) {
			return (TripdeckState)std::stoi(buffer.substr(HEADER_LENGTH + 2, 1));
		}

		inline TripdeckMediaOption _parseMediaOption(const std::string& buffer) {
			return (TripdeckMediaOption)std::stoi(buffer.substr(HEADER_LENGTH + 2, 1));
		}

		inline bool _containsMediaHashes(const std::string& buffer) {
			return buffer.substr(HEADER_LENGTH + 3, 1).compare("/") == 0;
		}
};

#endif