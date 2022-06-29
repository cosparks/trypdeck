#ifndef _TRIPDECK_BEHAVIOR_H_
#define _TRIPDECK_BEHAVIOR_H_

#include <vector>

#include "settings.h"
#include "td_util.h"
#include "Clock.h"
#include "Runnable.h"
#include "TripdeckMediaManager.h"
#include "InputManager.h"
#include "InputThreadedSerial.h"

using namespace td_util;

// length of any and all headers used for communication on the Serial network
#define HEADER_LENGTH 1
// startup notification structure: "nID"
// ID is unqiue identifier for sender
#define STARTUP_NOTIFICATION_HEADER 'n'
// startup notification structure: "cID/STATE/OPTION(/VIDEOHASH/LEDHASH)"
// STATE is TripdeckState to change to and OPTION contains information on how media should be played
// /VIDEOHASH/LEDHASH are optional arguments for video and led file IDs
// if VIDEOHASH or LEDHASH are zero, follower will select and play random media associated with that state
#define STATE_CHANGED_HEADER 'c'
// statis update message header: "uID/STATE/OPTION"
// sent during Follower's connected and wait phase -- generally to notify leader that follower is still connected
#define STATUS_UPDATE_HEADER 'u'
// statis update message header: "rID/STATE"
// sent by leader to confirm it has received status update from follower
#define CONNECTION_CONFIRMATION_HEADER 'r'
// message dictating that follower play media: "jID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define PLAY_MEDIA_HEADER 'j'
// message dictating that follower stop media: "sID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define STOP_MEDIA_HEADER 's'
// message dictating that follower pause media: "pID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define PAUSE_MEDIA_HEADER 'p'

#define MEDIA_OPTION_INDEX 5
#define DEFAULT_MEDIA_MESSAGE "x0/0/0"

/** @note if you add a new header, it must also be added to ValidHeaders array in Tripdeck.cpp */

// default rate for networking actions
#define DEFAULT_ACTION_INTERVAL 1000

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
		Command* _stateChangedDelegate = NULL;
		InputThreadedSerial* _serialInput = NULL;
		SerialInputDelegate* _serialInputDelegate = NULL;
		TripdeckStatus _status = { }; //  video and led media ids, current state and connection status
		int64_t _nextActionMillis = 0;

		bool _run = false;

		virtual void _handleSerialInput(InputArgs& args) = 0;
		bool _validateSerialMessage(const std::string& buffer);
		bool _validateHeader(char header);
		MediaHashes _parseMediaHashes(const std::string& buffer);
		const std::string _hashToHexString(uint32_t hash);

		template <class T>
		void _runTimedAction(T *object, void (T::*action)(void), int64_t interval = DEFAULT_ACTION_INTERVAL) {
			int64_t currentTime = Clock::instance().millis();
			if (currentTime >= _nextActionMillis) {
				_nextActionMillis = currentTime + interval;

				(object->*action)();
			}
		}

		inline const char _parseHeader(const std::string& buffer) {
			return buffer[0];
		}

		inline const std::string _parseId(const std::string& buffer) {
			return buffer.substr(HEADER_LENGTH, 1);
		}

		inline TripdeckState _parseState(const std::string& buffer) {
			return (TripdeckState)std::stoi(buffer.substr(HEADER_LENGTH + 2, 1));
		}

		inline TripdeckMediaOption _parseMediaOption(const std::string& buffer) {
			return (TripdeckMediaOption)std::stoi(buffer.substr(HEADER_LENGTH + 4, 1));
		}

		inline bool _containsMediaHashes(const std::string& buffer) {
			return buffer.substr(HEADER_LENGTH + 5, 1).compare("/") == 0;
		}
};

#endif