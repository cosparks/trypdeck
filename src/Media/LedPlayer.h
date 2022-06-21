#ifndef _LED_PLAYER_H_
#define _LED_PLAYER_H_

extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavformat/avformat.h"
}

#include <unordered_map>

#include "Apa102.h"
#include "MediaPlayer.h"

class LedPlayer : public MediaPlayer {
	public:
		LedPlayer(Apa102* apa102);
		~LedPlayer();
		void init() override;
		void run() override;
		void setCurrentMedia(uint32_t fileId, MediaPlaybackOption option = OneShot) override;
		uint32_t getCurrentMedia() override;
		uint32_t getNumMediaFiles() override;
		void play() override;
		void stop() override;
		void pause() override;

	private:
		uint32_t _currentMedia = 0;
		MediaPlaybackOption _playbackOption;
		std::unordered_map<uint32_t, uint32_t> _fileIdToData;
		Apa102* _apa102 = NULL;
		AVFormatContext* _formatContext =  NULL;
		AVCodecContext* _codecContext =  NULL;
		SwsContext* _swsContext = NULL;
		AVFrame* _frame;
		AVFrame* _frameRGB;
		AVRational _streamTimeBase = { 0, 0 };
		int32_t _streamId;
		int64_t _playStartTimeMicros = 0;
		int64_t _nextFrameTimeMicros = 0;
		bool _streamIsOpen = false;
		bool _mediaChanged = false;
		bool _streamRestarted = false;

		void _addMedia(uint32_t fileId) override;
		void _removeMedia(uint32_t fileId) override;
		void _updateMedia(uint32_t fileId) override;
		int32_t _getNextFrame();
		void _showNextFrame();
		void _openStream();
		void _closeStream();
};

#endif