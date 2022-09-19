#ifndef _TRIPDECK_STATE_H_
#define _TRIPDECK_STATE_H_

#include <stdint.h>

#include "MediaPlayer.h"

// Connecting and Connected == startup phase	// Wait == waiting for user input
// Pulled == chain has been pulled				// Reveal == card is being shown
enum TrypdeckState { Connecting, Connected, Wait, Pulled, Reveal, Unknown };

// Video == play video immediately				// Led == play led immediately
// Both == play both immediately				// None == wait to play media
enum TrypdeckMediaOption { None, Video, Led, Both };

struct TrypdeckStateChangedArgs {
	TrypdeckState state;
	uint32_t videoId;
	uint32_t ledId;
	TrypdeckMediaOption mediaOption;
	MediaPlayer::MediaPlaybackOption playbackOption;
};

struct TrypdeckStatus {
	uint32_t videoMedia;
	uint32_t ledMedia;
	TrypdeckMediaOption option;
	TrypdeckState state;
	int64_t lastTransmitMillis;
	bool connected;
};

#endif