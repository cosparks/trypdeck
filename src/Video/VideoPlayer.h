#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include <vlc/vlc.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "td_utility.h"

class VideoPlayer {
	public:
		enum VideoPlaybackOption { OneShot, Loop };

		VideoPlayer();
		~VideoPlayer();
		void init(const std::vector<std::string>& movies, const char *const *argv, int argc);
		void setCurrentMedia(const std::string& path, VideoPlaybackOption option = OneShot);
		std::string getCurrentMedia();
		void play();
		void playLoop();
		void stop();
		void pause();
	private:
		libvlc_instance_t* _instance = nullptr;
		libvlc_media_player_t* _mediaPlayer = nullptr;
		std::string _currentMedia;
		std::unordered_map<std::string, libvlc_media_t*> _mediaCache;
};

#endif