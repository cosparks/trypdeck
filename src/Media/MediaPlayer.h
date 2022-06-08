#ifndef _MEDIA_PLAYER_H_
#define _MEDIA_PLAYER_H_

#include "MediaListener.h"

class MediaPlayer : public MediaListener {
	public:
		enum MediaPlaybackOption { OneShot, Loop };
		enum MediaPlayerState { Play, Stop, Pause };
		MediaPlayer(const std::vector<std::string>& folders);
		~MediaPlayer();
		virtual void setCurrentMedia(uint32_t fileId, MediaPlaybackOption option = OneShot) = 0;
		virtual uint32_t getCurrentMedia() = 0;
		virtual uint32_t getNumMediaFiles() = 0;
		virtual void play() = 0;
		virtual void stop() = 0;
		virtual void pause() = 0;
		MediaPlayerState getState();
	protected:
		MediaPlayerState _state = Stop;
};

#endif