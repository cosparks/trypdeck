#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include <vlc/vlc.h>
#include <string>
#include <vector>
#include <map>

#include "media.h"

class VideoPlayer : public MediaListener {
	public:
		enum VideoPlaybackOption { OneShot, Loop };

		VideoPlayer(const std::vector<std::string>& folders);
		~VideoPlayer();
		void updateMedia(MediaChangedArgs args) override;
		void init(const std::vector<std::string>& movies, const char *const *argv, int argc);
		void setCurrentMedia(const std::string& path, VideoPlaybackOption option = OneShot);
		std::string getCurrentMedia();
		void play();
		void playLoop();
		void stop();
		void pause();
		const std::vector<std::string>& getMediaFolders();
	private:
		libvlc_instance_t* _instance = nullptr;
		libvlc_media_player_t* _mediaPlayer = nullptr;
		libvlc_media_list_player_t* _mediaListPlayer = nullptr;
		libvlc_media_list_t* _mediaList = nullptr;
		std::string _currentMedia;
		std::map<std::string, libvlc_media_t*> _mediaCache;
		std::vector<std::string> _mediaFolders;

		int32_t _insertMedia(libvlc_media_t* media, int32_t i);
};

#endif