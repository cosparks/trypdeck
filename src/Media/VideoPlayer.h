#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include <vlc/vlc.h>
#include <queue>
#include <unordered_map>

#include "MediaPlayer.h"

class VideoPlayer : public MediaPlayer {
	public:
		VideoPlayer();
		~VideoPlayer();
		void init() override;
		void run() override;
		void setCurrentMedia(uint32_t fileId, MediaPlaybackOption option = OneShot) override;
		uint32_t getCurrentMedia() override;
		uint32_t getNumMediaFiles() override;
		void play() override;
		void stop() override;
		void pause() override;

	private:
		uint32_t _currentMedia;
		std::unordered_map<uint32_t, int32_t> _fileIdToIndex;
		std::queue<uint32_t> _emptyIndices;
		// lib vlc
		libvlc_instance_t* _instance = NULL;
		libvlc_media_list_player_t* _mediaListPlayer = NULL;
		int32_t _mediaListSize = 0;
		libvlc_media_list_t* _mediaList = NULL;

		void _addMedia(uint32_t fileId) override;
		void _removeMedia(uint32_t fileId) override;
		void _updateMedia(uint32_t fileId) override;
		void _createAndInsertMedia(uint32_t fileId, int32_t i);
		void _removeMediaAtIndex(int32_t i);
		
};

#endif