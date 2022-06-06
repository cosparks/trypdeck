#ifndef _VIDEO_PLAYER_H_
#define _VIDEO_PLAYER_H_

#include <vlc/vlc.h>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include "media.h"

class VideoPlayer : public MediaListener {
	public:
		enum VideoPlaybackOption { OneShot, Loop };

		VideoPlayer(const std::vector<std::string>& folders);
		~VideoPlayer();
		void addFileIds(const std::vector<uint32_t>& ids);
		void updateMedia(const MediaChangedArgs& args) override;
		void init(const char *const *argv, int argc);
		void setCurrentMedia(uint32_t fileId, VideoPlaybackOption option = OneShot);
		uint32_t getCurrentMedia();
		void playOneShot();
		void playLoop();
		void stop();
		void pause();
		const std::vector<std::string>& getMediaFolders();
	private:
		uint32_t _currentMedia;
		std::vector<std::string> _mediaFolders;
		std::unordered_map<uint32_t, int32_t> _fileIdToIndex;
		std::queue<uint32_t> _emptyIndices;

		// lib vlc
		libvlc_instance_t* _instance = nullptr;
		libvlc_media_list_player_t* _mediaListPlayer = nullptr;
		int32_t _mediaListSize = 0;
		libvlc_media_list_t* _mediaList = nullptr;

		void _addFile(uint32_t fileId);
		void _removeFile(uint32_t fileId);
		void _updateMediaForFile(uint32_t fileId);
		void _createAndInsertMedia(uint32_t fileId, int32_t i);
		void _removeMediaAtIndex(int32_t i);
};

#endif