#ifndef _TRIPDECK_STATE_H_
#define _TRIPDECK_STATE_H_

#include <stdint.h>

// Connecting and Connected == startup phase	// Wait == waiting for user input
// Pulled == chain has been pulled				// Reveal == card is being shown
enum TripdeckState { Connecting, Connected, Wait, Pulled, Reveal, Unknown };

// Video == play video immediately				// Led == play led immediately
// Both == play both immediately				// None == wait to play media
enum TripdeckMediaOption { None, Video, Led, Both };

struct TripdeckStateChangedArgs {
	TripdeckState newState;
	uint32_t videoId;
	uint32_t ledId;
	TripdeckMediaOption mediaOption;
	bool loop;
};

struct TripdeckStatus {
	uint32_t videoMedia;
	uint32_t ledMedia;
	TripdeckMediaOption option;
	TripdeckState state;
	bool connected;
};

#endif