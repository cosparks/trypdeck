#include <iostream>
#include "settings.h"

#include "Clock.h"
#include "Index.h"
#include "LedPlayer.h"

LedPlayer::LedPlayer(const std::vector<std::string>& folders, Apa102* apa102) : MediaPlayer(folders) {
	_apa102 = apa102;
}

LedPlayer::~LedPlayer() { }

void LedPlayer::init() {
	if (gpioInitialise() < 0)
		throw std::runtime_error("Error: PI GPIO Initialization failed");

	_apa102->init(0, SPI_BAUD, 0);
	_apa102->clear();
	_apa102->show();
}

void LedPlayer::run() {
	if (_state == Play) {
		if (Clock::instance().micros() > _nextFrameTimeMicros) {
			_showNextFrame();

			if (!_getNextFrame()) {
				// TODO: handle end of stream
			}
		}
	}
}

void LedPlayer::setCurrentMedia(uint32_t fileId, MediaPlaybackOption option) {
	if (_fileIdToData.find(fileId) == _fileIdToData.end()) {
		throw std::runtime_error("Error: LedPlayer does not contain reference to file: " + Index::instance().getSystemPath(fileId));
	}

	_currentMedia = fileId;
	_playbackOption = option;

	if (_state == Play || _state == Pause)
		stop();
	
	_openFormatContext();
	_openCodecContext();
	_openSwsContext();
	_updateFrameContext();
}

uint32_t LedPlayer::getCurrentMedia() {
	return _currentMedia;
}

uint32_t LedPlayer::getNumMediaFiles() {
	return _fileIdToData.size();
}

void LedPlayer::play() {
	if (_formatContext == NULL)
		throw std::runtime_error("Error: Must set current media on LedPlayer before calling play()");

	if (_state == Pause) {
		_playStartTimeMicros = Clock::instance().micros() - (_nextFrameTimeMicros - _playStartTimeMicros);
		if (_frameRGB != NULL)
			_showNextFrame();
	} else {
		_playStartTimeMicros = Clock::instance().micros();
	}
	_state = Play;

	if (!_getNextFrame()) {
		// TODO: handle end of stream
		throw std::runtime_error("TEST ERROR: Unable to grab next frame");
	}
}

void LedPlayer::stop() {
	_state = Stop;
	_closeStream();

	_apa102->clear();
	_apa102->show();
}

void LedPlayer::pause() {
	if (_state == Stop || _state == Pause)
		return;

	_state = Pause;
}

void LedPlayer::_addMedia(uint32_t fileId) {
	if (_fileIdToData.find(fileId) == _fileIdToData.end()) {
		_fileIdToData[fileId] = 0;
	}
}

void LedPlayer::_removeMedia(uint32_t fileId) {
	if (_fileIdToData.find(fileId) != _fileIdToData.end()) {
		_fileIdToData.erase(fileId);
	}
}

void LedPlayer::_updateMedia(uint32_t fileId) {
	// do nothing
}

bool LedPlayer::_getNextFrame() {
	while (av_read_frame(_formatContext, &_avPacket) >= 0) {
		if (_avPacket.size == 0)
			return false;

		// This function might fail. Ignore and continue
		int32_t ret = avcodec_send_packet(_codecContext, &_avPacket);
		if (ret < 0)
			continue;

		// get uncompressed frame
		ret = avcodec_receive_frame(_codecContext, _frame);

		if (ret < 0) {
			// Sometimes we cannot get a new frame, continue in this case
			if (ret == AVERROR(EAGAIN))
				continue;

			throw std::runtime_error("Error: avcodec_receive_frame returned < 0");
		}

		// Convert frame data to RGB
		sws_scale(_swsContext, (uint8_t const * const *)_frame->data, _frame->linesize, 0, _frame->height, _frameRGB->data, _frameRGB->linesize);
		_nextFrameTimeMicros = _playStartTimeMicros + _frame->best_effort_timestamp * (1000000L / _streamTimeBase.den);
		break;
	}

	return true;
}

void LedPlayer::_showNextFrame() {
	int32_t width = _apa102->getWidth();
	int32_t height = _apa102->getHeight();
	int32_t pixelsPerLedX = _frameRGB->width / width;
	int32_t pixelsPerLedY = _frameRGB->height / height;
	int32_t offsetX = 0;
	int32_t offsetY = 0;

	// Write pixel data
	for(int32_t y = 0; y < height; y++) {
		for (int32_t x = 0; x < width; x++) {
			uint8_t* ptr = _frameRGB->data[0] + (y * pixelsPerLedY + offsetY) * _frameRGB->linesize[0] + 3 * (x * pixelsPerLedX + offsetX);
			_apa102->setPixel(Pixel { PIXEL_BRIGHTNESS, ptr[0], ptr[1], ptr[2] }, Point { x, y });
		}
	}
	_apa102->show();
}

void LedPlayer::_openFormatContext() {
	if (avformat_open_input(&_formatContext, Index::instance().getSystemPath(_currentMedia).c_str(), NULL, NULL) < 0)
		throw std::runtime_error("Error: Unable to open format context for file: " + Index::instance().getSystemPath(_currentMedia));

	if (avformat_find_stream_info(_formatContext, NULL) < 0)
		throw std::runtime_error("Error: Unable to find stream info for file: " + Index::instance().getSystemPath(_currentMedia));
}

void LedPlayer::_openCodecContext() {
	int32_t streamId;
	AVStream *stream;
	AVCodecParameters *codecParams = NULL;
	AVCodec *decoder = NULL;
	streamId = av_find_best_stream(_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (streamId < 0) {
		throw std::runtime_error("Error: Unable to find stream in file: " + Index::instance().getSystemPath(_currentMedia));
	}
	else {
		stream = _formatContext->streams[streamId];
		_streamTimeBase = stream->time_base;

		// find decoder for the stream
		codecParams = stream->codecpar;
		decoder = avcodec_find_decoder(codecParams->codec_id);

		if (!decoder)
			throw std::runtime_error("Error: Unable to find decoder for file: " + Index::instance().getSystemPath(_currentMedia));

		_codecContext = avcodec_alloc_context3(decoder);
		avcodec_parameters_to_context(_codecContext, codecParams);

		if (avcodec_open2(_codecContext, decoder, NULL) < 0)
			throw std::runtime_error("Error: Unable to open decoder for file: " + Index::instance().getSystemPath(_currentMedia));
	}
}

void LedPlayer::_openSwsContext() {
	_swsContext = sws_getContext(_codecContext->width, _codecContext->height,
		AVPixelFormat(_codecContext->pix_fmt), _codecContext->width, _codecContext->height,
		AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

	if (_swsContext == NULL)
		throw std::runtime_error("Error: unable to open sws context for current media");
}

void LedPlayer::_updateFrameContext() {
	av_init_packet(&_avPacket);
	_frame = av_frame_alloc();
	_frameRGB = av_frame_alloc();
	_frameRGB->format = AV_PIX_FMT_RGB24;
	_frameRGB->width = _codecContext->width;
	_frameRGB->height = _codecContext->height;

	if (av_frame_get_buffer(_frameRGB, 32) < 0) {
		throw std::runtime_error("Error: Problem allocating AVFrame buffer for LedPlayer::_frameRGB");
	}
}

void LedPlayer::_closeStream() {
	if (_swsContext != NULL)
		sws_freeContext(_swsContext);
	if (_codecContext != NULL)
		avcodec_free_context(&_codecContext);
	if (_formatContext != NULL)	
		avformat_close_input(&_formatContext);
	if (_frameRGB != NULL)
		av_frame_free(&_frameRGB);
	if (_frame != NULL)
		av_frame_free(&_frame);

	av_packet_unref(&_avPacket);
}