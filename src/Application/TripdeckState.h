#ifndef _TRIPDECK_STATE_H_
#define _TRIPDECK_STATE_H_

#include <stdint.h>

// Connecting and Connected == startup phase	// Wait == waiting for user input
// Pulled == chain has been pulled				// Reveal == card is being shown
enum TripdeckState { Connecting, Connected, Wait, Pulled, Reveal, Unknown };
enum TripdeckMediaOption { Video, Led, Both, None };

struct TripdeckStateChangedArgs {
	TripdeckState newState;
	uint32_t videoId;
	uint32_t ledId;
	bool syncVideo;
	bool syncLeds;
	bool loop;
	TripdeckMediaOption mediaOption;
};

struct TripdeckStatus {
	uint32_t videoMedia;
	uint32_t ledMedia;
	TripdeckState state;
	bool connected;
};

#endif