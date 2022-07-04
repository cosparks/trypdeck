#ifndef _MEDIA_PLAYER_H_
#define _MEDIA_PLAYER_H_

#include "MediaListener.h"
#include "td_util.h"

using namespace td_util;

class MediaPlayer : public MediaListener {
	public:
		enum MediaPlaybackOption { OneShot, Loop, Cycle };
		enum MediaPlayerState { Play, Stop, Pause };
		MediaPlayer();
		~MediaPlayer();
		virtual void setCurrentMedia(uint32_t fileId, MediaPlaybackOption option = OneShot) = 0;
		virtual uint32_t getCurrentMedia() = 0;
		virtual uint32_t getNumMediaFiles() = 0;
		virtual void play() = 0;
		virtual void stop() = 0;
		virtual void pause() = 0;
		virtual bool containsMedia(uint32_t fileId) = 0;
		MediaPlayerState getState();
		bool isPlaying();
		void setPlaybackCompleteDelegate(Command* delegate);

		struct PlaybackCompleteArgs {
			MediaPlaybackOption playbackOption;
			uint32_t currentMedia;
		};

	protected:
		MediaPlayerState _state = Stop;
		uint32_t _currentMedia = 0;
		MediaPlaybackOption _currentOption;
		Command* _playbackCompleteDelegate = NULL;

		void _onPlaybackComplete();
};

#endif