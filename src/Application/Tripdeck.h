#ifndef _TRIPDECK_BEHAVIOR_H_
#define _TRIPDECK_BEHAVIOR_H_

#include <unordered_map>
#include <vector>

#include "settings.h"
#include "td_util.h"
#include "Clock.h"
#include "Runnable.h"
#include "TripdeckMediaManager.h"
#include "InputManager.h"
#include "InputThreadedSerial.h"

using namespace td_util;

/** HEADERS 
 * @note if you add a new header, it must also be added to ValidHeaders array in Tripdeck.cpp
**/

// startup notification structure: "cID/STATE/OPTION/LOOP(/VIDEOHASH/LEDHASH)"
// STATE is TripdeckState to change to and OPTION/LOOP contains information on how media should be played
// VIDEOHASH/LEDHASH are optional arguments for video and led file IDs
// if VIDEOHASH or LEDHASH are zero, follower will select and play random media associated with that state
#define STATE_CHANGED_HEADER 'c'
// startup notification structure: "nID"
// ID is unqiue identifier for sender
#define STARTUP_NOTIFICATION_HEADER 'n'
// status update message header: "uID/STATE/OPTION/LOOP(/VIDEOHASH/LEDHASH)"
// sent during Follower's connected and wait phase -- generally to notify leader that follower is still connected
#define STATUS_UPDATE_HEADER 'u'
// message dictating that follower play media: "jID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define PLAY_MEDIA_HEADER 'j'
// message dictating that follower stop media: "sID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define STOP_MEDIA_HEADER 's'
// message dictating that follower pause media: "pID/STATE/OPTION"
// OPTION is TripdeckMediaOption
#define PAUSE_MEDIA_HEADER 'p'
// play media from state folder message: "mID/STATE/OPTION/LOOP(/VIDEOHASH/LEDHASH)"
// allows Leader to send message to follower indicating that it should play media from a folder not associated with its current state
#define PLAY_MEDIA_FROM_STATE_FOLDER_HEADER 'm'
// reset message structure: "r/all"
// triggers reboot of all pis currently connected on network
#define SYSTEM_RESET_HEADER 'r'
// reset message structure: "x/all"
// triggers reboot of all pis currently connected on network
#define SYSTEM_SHUTDOWN_HEADER 'x'

/** MESSAGE METADATA 
 * @note you must change these values if you make any changes to message format
**/
#define HEADER_LENGTH 1
#define ID_INDEX 1
#define STATE_INDEX 3
#define MEDIA_OPTION_INDEX 5
#define LOOP_INDEX 7
#define HASH_INDEX 9
#define DEFAULT_MESSAGE "x0/0/0/0"
#define RESET_MESSAGE "r/all"

// default rate for networking actions
#define DEFAULT_ACTION_INTERVAL DEFAULT_PING_INTERVAL

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

	protected:
		struct MediaHashes {
			uint32_t videoHash;
			uint32_t ledHash;
		};
		
		TripdeckMediaManager* _mediaManager = NULL;
		InputManager* _inputManager = NULL;
		Serial* _serial = NULL;
		TripdeckStatus _status = { }; //  video and led media ids, current state and connection status
		int64_t _nextActionMillis = 0;
		bool _run = false;

		virtual void _handleSerialInput(InputArgs& args) = 0;
		bool _validateSerialMessage(const std::string& buffer);
		bool _validateHeader(char header);
		MediaHashes _parseMediaHashes(const std::string& buffer);
		const std::string _hashToHexString(uint32_t hash);
		void _updateStatusFromStateArgs(TripdeckStateChangedArgs& args);
		void _populateStateArgsFromBuffer(const std::string& buffer, TripdeckStateChangedArgs& args);
		std::string _populateBufferFromStateArgs(const TripdeckStateChangedArgs& args, char header = '0', char id = '0');
		void _reset();
		void _shutdown();

		template <class T>
		void _runTimedAction(T *object, void (T::*action)(void), int64_t interval = DEFAULT_ACTION_INTERVAL) {
			int64_t currentTime = Clock::instance().millis();
			if (currentTime >= _nextActionMillis) {
				_nextActionMillis = currentTime + interval;

				(object->*action)();
			}
		}

		inline char _parseHeader(const std::string& buffer) {
			return buffer[0];
		}

		inline char _parseId(const std::string& buffer) {
			return buffer[ID_INDEX];
		}

		inline TripdeckState _parseState(const std::string& buffer) {
			return (TripdeckState)(buffer[STATE_INDEX] - '0');
		}

		inline TripdeckMediaOption _parseMediaOption(const std::string& buffer) {
			return (TripdeckMediaOption)(buffer[MEDIA_OPTION_INDEX] - '0');
		}

		inline bool _parseLoop(const std::string& buffer) {
			return buffer[LOOP_INDEX] == '1';
		}

		inline bool _containsMediaHashes(const std::string& buffer) {
			return buffer.size() >= HASH_INDEX && buffer[HASH_INDEX - 1] == '/';
		}

		inline char _singleDigitIntToChar(int32_t num) {
			return (char)('0' + num);
		}

		class SerialInputDelegate : public Command {
			public:
				SerialInputDelegate(Tripdeck* owner);
				~SerialInputDelegate();
				void execute(CommandArgs args) override;
			private:
				Tripdeck* _owner;
		};
};

#endif