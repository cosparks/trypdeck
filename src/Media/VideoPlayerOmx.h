#ifndef _VIDEO_PLAYER_OMX_H_
#define _VIDEO_PLAYER_OMX_H_

#include <thread>
#include <mutex>
#include <unordered_map>

#include "MediaPlayer.h"

class VideoPlayerOmx : public MediaPlayer {
	public:
		VideoPlayerOmx();
		~VideoPlayerOmx();
		void init() override { }
		void run() override { }
		void setCurrentMedia(uint32_t fileId, MediaPlaybackOption option = OneShot) override;
		uint32_t getCurrentMedia() override;
		uint32_t getNumMediaFiles() override;
		void play() override;
		void stop() override;
		void pause() override;
		bool containsMedia(uint32_t fileId) override;

	private:
		std::string _omxplayerArgs;
		std::mutex _stateMutex;
		std::unordered_map<uint32_t, std::string> _fileIdToSystemPath;

		void _addMedia(uint32_t fileId) override;
		void _removeMedia(uint32_t fileId) override;
		void _updateMedia(uint32_t fileId) override;
		void _playInternal();
		void _threadExitCallback();
		void _onThreadExit(void (VideoPlayerOmx::*callback)(void));

};

#endif