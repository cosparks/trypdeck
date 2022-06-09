#include <iostream>
#include "settings.h"

#include "Clock.h"
#include "Index.h"
#include "LedPlayer.h"

LedPlayer::LedPlayer(const std::vector<std::string>& folders, Apa102* apa102) : MediaPlayer(folders) {
	_apa102 = apa102;
}

LedPlayer::~LedPlayer() {
	#if not ENABLE_DEBUG
	_apa102->clear();
	_apa102->show();
	#endif
}

void LedPlayer::init() {
	#if not ENABLE_DEBUG
	if (gpioInitialise() < 0)
		throw std::runtime_error("Error: PI GPIO Initialization failed");
	#endif
	
	_apa102->init(0, SPI_BAUD, 0);

	#if not ENABLE_DEBUG
	_apa102->clear();
	_apa102->show();
	#endif
}

void LedPlayer::run() {
	if (_state == Play) {
		if (!_streamIsOpen)
			throw std::runtime_error("Error: LedPlayer::play called without first opening stream");

		if (Clock::instance().micros() > _nextFrameTimeMicros) {
			_showNextFrame();

			if (_getNextFrame() < 0) {
				stop();
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
	_mediaChanged = true;
}

uint32_t LedPlayer::getCurrentMedia() {
	return _currentMedia;
}

uint32_t LedPlayer::getNumMediaFiles() {
	return _fileIdToData.size();
}

void LedPlayer::play() {
	if (!_currentMedia)
		throw std::runtime_error("Error: LedPlayer::_currentMedia has either not been set, or has been removed!");
	
	// return if we are being asked to play media which is already playing
	if (_state == Play && !_mediaChanged)
		return;

	bool openStream = true;

	// if we have opened a stream, paused and haven't changed _currentMedia, start playback from same point and don't reopen stream
	//		otherwise if the stream is open, close it and restart
	if (_state == Pause && _streamIsOpen && !_mediaChanged) {
		openStream = false;
		_playStartTimeMicros = Clock::instance().micros() - (_nextFrameTimeMicros - _playStartTimeMicros);
		_showNextFrame();
	} else if (_streamIsOpen) {
		_closeStream();
	}

	if (openStream) {
		_openStream();
		_playStartTimeMicros = Clock::instance().micros();
	}

	_mediaChanged = false;
	_state = Play;

	if (_getNextFrame() < 0) {
		stop();
	}
}

void LedPlayer::stop() {
	if (_state == Stop)
		return;

	_state = Stop;

	#if not ENABLE_DEBUG
	_apa102->clear();
	_apa102->show();
	#endif
}

void LedPlayer::pause() {
	if (_state == Pause || _state == Stop)
		return;

	_state = Pause;
}

void LedPlayer::_addMedia(uint32_t fileId) {
	if (_fileIdToData.find(fileId) == _fileIdToData.end()) {
		_fileIdToData[fileId] = 0;

		// TODO: REMOVE (TEMP BEHAVIOR FOR TESTING)
		if (_streamIsOpen) {
			setCurrentMedia(fileId, MediaPlaybackOption::Loop);

			if (_state == MediaPlayerState::Play)
				play();
		}
	}
}

void LedPlayer::_removeMedia(uint32_t fileId) {
	if (_fileIdToData.find(fileId) != _fileIdToData.end()) {
		// if current media file has been removed and LedPlayer is playing it, stop
		if (_currentMedia == fileId && _state != Stop) {
			_currentMedia = 0;
			stop();
			_closeStream();
		}

		_fileIdToData.erase(fileId);
	}
}

void LedPlayer::_updateMedia(uint32_t fileId) {
	if (_currentMedia == fileId) {
		_mediaChanged == true;

		if (_state == MediaPlayerState::Play)
			play();
	}
}

int32_t LedPlayer::_getNextFrame() {
	int32_t ret;
	AVPacket packet = { };
	av_init_packet(&packet);
	bool streamRestarted = false;

	while (true) {
		ret = av_read_frame(_formatContext, &packet);

		if (ret == AVERROR_EOF) {
			if (_playbackOption == MediaPlaybackOption::Loop) {
				// reset avformat context and and continue loop
				AVStream* stream = _formatContext->streams[_streamId];
				avio_seek(_formatContext->pb, 0, SEEK_SET);
				avformat_seek_file(_formatContext, _streamId, 0, 0, stream->duration, 0);

				_playStartTimeMicros = Clock::instance().micros();
				streamRestarted = true;
				continue;
			} else {
				ret = -1;
				break;
			}
		}

		if (packet.size == 0)
			break;

		// This function might fail. Ignore and continue
		int32_t ret = avcodec_send_packet(_codecContext, &packet);
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

		// sometimes when a stream is restarted, the first frame will have an incorrect timestamp. branching here fixes that
		_nextFrameTimeMicros = streamRestarted ? _playStartTimeMicros : _playStartTimeMicros + _frame->best_effort_timestamp * (1000000L / _streamTimeBase.den);
		break;
	}
	av_packet_unref(&packet);
	return ret;
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

	// TODO: REMOVE DEBUG CODE

	#if not ENABLE_DEBUG
	_apa102->show();
	#endif
}

void LedPlayer::_openStream() {
	_openFormatContext();
	_openCodecContext();
	_openSwsContext();
	_updateFrameContext();
	_streamIsOpen = true;
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

	_streamIsOpen = false;
}

void LedPlayer::_openFormatContext() {
	if (avformat_open_input(&_formatContext, Index::instance().getSystemPath(_currentMedia).c_str(), NULL, NULL) < 0)
		throw std::runtime_error("Error: Unable to open format context for file: " + Index::instance().getSystemPath(_currentMedia));

	if (avformat_find_stream_info(_formatContext, NULL) < 0)
		throw std::runtime_error("Error: Unable to find stream info for file: " + Index::instance().getSystemPath(_currentMedia));
}

void LedPlayer::_openCodecContext() {
	AVStream *stream;
	AVCodecParameters *codecParams = NULL;
	AVCodec *decoder = NULL;
	_streamId = av_find_best_stream(_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (_streamId < 0) {
		throw std::runtime_error("Error: Unable to find stream in file: " + Index::instance().getSystemPath(_currentMedia));
	}
	else {
		stream = _formatContext->streams[_streamId];
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
	_frame = av_frame_alloc();
	_frameRGB = av_frame_alloc();
	_frameRGB->format = AV_PIX_FMT_RGB24;
	_frameRGB->width = _codecContext->width;
	_frameRGB->height = _codecContext->height;

	if (av_frame_get_buffer(_frameRGB, 32) < 0) {
		throw std::runtime_error("Error: Problem allocating AVFrame buffer for LedPlayer::_frameRGB");
	}
}