#include <iostream>
#include <fstream>
#include <stdio.h>
#include <thread>
#include <pigpio.h>
#include <vlc/vlc.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

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

#include "Apa102.h"
#include "Clock.h"

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

using namespace std;

// PROGRAM MACROS
#define OMX_ARGS "omxplayer /home/trypdeck/projects/tripdeck_basscoast/src/video/numa.m4v"
#define VLC_VIDEO_PATH "video/music.mp4"
#define TRANSCODE_VIDEO_PATH "video/numa.m4v"

#define RUN_AV_TRANSCODING 0
#define PLAY_OMX 0
#define SAVE_RGB_FRAMES 0
#define RUN_LEDS 0
#define RUN_BRIGHTNESS_TEST 0
#define RUN_FILL_TEST 0
#define RUN_SERIAL_NETWORKING 1
#define SERIAL_ID 11
#define INITIALIZE_GPIO RUN_LEDS | RUN_SERIAL_NETWORKING

#define SERIAL_BAUD 9600
#define SPI_BAUD 6000000

#define TERMINATE_PROGRAM 0
#define RUN_INTERVAL 100000
#define PRINT_INTERVAL 1000
#define LED_INTERVAL 50
#define MATRIX_WIDTH 60
#define MATRIX_HEIGHT 40

#define INBUF_SIZE 4096

bool initializeGpio() {
	if (gpioInitialise() < 0) {
		cout << "PI GPIO Initialization failed" << endl;
		return false;
	}
	else {
		cout << "PI GPIO Initialization successful" << endl;
		return true;
	}

	// set up SPI (10 is SPI MOSI and 11 is SPI clock)
	if (!gpioSetMode(10, PI_OUTPUT)) {
		cout << "Unable to set SPI MOSI mode" << endl;
	}

	if (!gpioSetMode(11, PI_OUTPUT)) {
		cout << "Unable to set SPI clock mode";
	}
}

void play_video() {
	system(OMX_ARGS);
}

void initializeVlc() {
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	const char* args[] = { "-v", "-I", "dummy", "--fullscreen", "--no-osd", "--no-audio", "--x11-display", ":0" };
	int numArgs = sizeof(args) / sizeof(args[0]);
	inst = libvlc_new(numArgs, args);

	/* Create a new item */
	//m = libvlc_media_new_location(inst, "video/climbing.m4v");
	m = libvlc_media_new_path (inst, VLC_VIDEO_PATH);
	
	/* Create a media player playing environement */
	mp = libvlc_media_player_new_from_media(m);

	this_thread::sleep_for(chrono::seconds(1));

	/* No need to keep the media now */
	libvlc_media_release(m);

	// libvlc_video_take_snapshot(mp, 0, "snapshots/new.png", 0, 0);
	// libvlc_media_player_set_xwindow(mp, 0);

	/* play the media_player */
	libvlc_media_player_play(mp);

	// /* Let it play a bit */
	this_thread::sleep_for(chrono::seconds(40));

	// /* Stop playing */
	libvlc_media_player_stop(mp);

	// /* Free the media_player */
	libvlc_media_player_release(mp);

	libvlc_release(inst);
}

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
static AVStream *video_stream = NULL, *audio_stream = NULL;
static const char *src_filename = NULL;
static int video_dst_bufsize;
static int video_stream_idx = -1, audio_stream_idx = -1;
static AVFrame *frame = NULL;
static int video_frame_count = 0;

const char *media_type_string(enum AVMediaType media_type)
{
	switch (media_type) {
	case AVMEDIA_TYPE_VIDEO:      return "video";
	case AVMEDIA_TYPE_AUDIO:      return "audio";
	case AVMEDIA_TYPE_DATA:       return "data";
	case AVMEDIA_TYPE_SUBTITLE:   return "subtitle";
	case AVMEDIA_TYPE_ATTACHMENT: return "attachment";
	default:                      return "unknown";
	}
}

static int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret;
	AVStream *st;
	AVCodecParameters *dec_param = NULL;
	AVCodec *dec = NULL;
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
		media_type_string(type), src_filename);
		return ret;
	}
	else {
		*stream_idx = ret;
		st = fmt_ctx->streams[*stream_idx];
		// find decoder for the stream
		dec_param = st->codecpar;
		dec = avcodec_find_decoder(dec_param->codec_id);
		if (!dec) {
		fprintf(stderr, "Failed to find %s codec\n",
			media_type_string(type));
		return ret;
		}
		video_dec_ctx = avcodec_alloc_context3(dec);
		avcodec_parameters_to_context(video_dec_ctx, dec_param);
		if ((ret = avcodec_open2(video_dec_ctx, dec, NULL)) < 0) {
		fprintf(stderr, "Failed to open %s codec\n",
			media_type_string(type));
		return ret;
		}
	}
	return 0;
}

void saveFrame(AVFrame* frameRGB, int frameNumber) {
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "gen/frame%d.ppm", frameNumber);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", MATRIX_WIDTH, MATRIX_HEIGHT);

	printf("Frame %d\n", frameNumber);

	int pixelsPerLedWide = frameRGB->width / MATRIX_WIDTH;
	int pixelsPerLedHigh = frameRGB->height / MATRIX_HEIGHT;
	int bufferSize = MATRIX_WIDTH * 3;
	uint8_t buffer[bufferSize] = { };

	// Write pixel data
	for(y = 0; y < MATRIX_HEIGHT; y++) {
		for (int x = 0; x < bufferSize; x += 3) {
			uint8_t* ptr = frameRGB->data[0] + y * pixelsPerLedHigh * frameRGB->linesize[0] + (x * pixelsPerLedWide);
			buffer[x] = ptr[0];
			buffer[x + 1] = ptr[1];
			buffer[x + 2] = ptr[2];
		}
		fwrite(buffer, 1, bufferSize, pFile);
	}

	// Close file
	fclose(pFile);
}

int av_read_frame_log_time(AVFormatContext* context, AVPacket* packet, int64_t& time) {
	// cout << "Program time before read frame: " << Clock::instance().millis() << endl;
	time = Clock::instance().micros();
	return av_read_frame(context, packet);
}

static void video_decode_example()
{
	AVCodecContext *c = NULL;
	AVPacket avpkt;

	av_init_packet(&avpkt);

	// printf("Video decoding\n");
	int iFrame = -1;
	int ret = 0;
	frame = 0;
	AVFrame *frame = av_frame_alloc();
	uint8_t* imagebuffer = NULL;

	// PERFORMANCE Testing
	int64_t microBuffer[10000];
	int i = 0;
	int64_t start = Clock::instance().seconds();

	// Read all the frames
	while (av_read_frame_log_time(fmt_ctx, &avpkt, microBuffer[i]) >= 0) {
		if (avpkt.size == 0)
			break;
		// fprintf(stderr, "Video frame size %d\r\n", avpkt.size);
		// cout << "Program time after read frame: " << Clock::instance().millis() << endl;

		// This function might fail because of parameter set packets, just ignore and continue
		ret = avcodec_send_packet(video_dec_ctx, &avpkt);
		if (ret < 0) {
			fprintf(stderr, "avcodec_send_packet ret < 0\n");
			continue;
		}

		//cout << "Program time before avcodec receive frame: " << Clock::instance().millis() << endl;
		// Receive the uncompressed frame back
		ret = avcodec_receive_frame(video_dec_ctx, frame);
		
		//cout << "Program time before frame converted to RGB: " << Clock::instance().millis() << endl;

		if (ret < 0) {
			// Sometimes we cannot get a new frame, continue in this case
			if (ret == AVERROR(EAGAIN))
				continue;

			fprintf(stderr, "avcodec_receive_frame ret < 0\n");
			break;
		}

		// Convert frame data to RGB
		SwsContext* swsContext = sws_getContext(frame->width, frame->height, AVPixelFormat(frame->format), frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		AVFrame* frameRGB = av_frame_alloc(); frameRGB->format = AV_PIX_FMT_RGB24; frameRGB->width = frame->width; frameRGB->height = frame->height;
		av_frame_get_buffer(frameRGB, 32);
		sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
		sws_freeContext(swsContext);
		
		#if SAVE_RGB_FRAMES
		iFrame++;
		if ((iFrame % 4 == 0)) {
			saveFrame(frameRGB, iFrame);
		}
		#endif

		av_frame_free(&frameRGB);
		i++;
		microBuffer[i++] = Clock::instance().micros();
	}

	// TOTAL AND AVERAGE FRAME TIMES
	int64_t end = Clock::instance().seconds();
	int64_t j;
	int64_t sum = 0;
	int64_t count = 0;
	for (j = 0; microBuffer[j]; j+=2) {
		if (!microBuffer[j+1]) {
			break;
		}
		sum += (microBuffer[j + 1] - microBuffer[j]);
		count++;
	}

	if (!microBuffer[j-1]) {
		j--;
	}

	cout << "Total Transcoding time in seconds is: " << end - start << endl;
	cout << "The average time to read a frame in microseconds is: " << sum / count << endl;
	cout << "The average time to read a frame in milliseconds is: " << (sum / count) / 1000 << endl;
	cout << "Average frames per second: " << 1000 / ((sum / count) / 1000) << endl;

	delete imagebuffer;
	fprintf(stderr, "out of loop av_read_frame\n");
	avcodec_close(video_dec_ctx);
	av_free(video_dec_ctx);
	av_frame_free(&frame);
}

int runAv() {
	/* register all the codecs */

	// ffmpeg 4.0 deprecated registering, might still need for libav
	//fprintf(stderr, "Register everything\r\n");
	//av_register_all();
	//avcodec_register_all();
	
	src_filename = TRANSCODE_VIDEO_PATH;

	int64_t start = Clock::instance().micros();
	// open input file, and allocate format context
	if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", src_filename);
		exit(1);
	}

	// retrieve stream information
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
		exit(1);
	}

	// Open video context
	if (open_codec_context(&video_stream_idx, fmt_ctx, AVMEDIA_TYPE_VIDEO) < 0) {
		fprintf(stderr, "open_codec_context failed\n");
		exit(1);
	}
	int64_t end = Clock::instance().micros();
	cout << "Time to open video stream in microseconds: " << end - start << endl;
	cout << "Time to open video stream in milliseconds: " << (end - start) / 1000 << endl;

	video_decode_example();
	return 0;
}

int main(int argv, char** argc) {
	int64_t lastPrintTime = Clock::instance().millis();

	#if INITIALIZE_GPIO
	if (!initializeGpio()) {
		return -1;
	}
	#endif

	#if RUN_SERIAL_NETWORKING
	int error = i2cOpen(0, SERIAL_ID, 0);
	if (error < 0) {
		cout << "Unable to open I2C port, terminating with error " << error << endl;
		return -1;
	}
	uint32_t handle = error;
	int64_t last_serial_publish = Clock::instance().millis();
	#endif

	#if PLAY_OMX
	thread omx(play_video);
	omx.detach();
	#endif

	#if RUN_AV_TRANSCODING
	thread avCodec(runAv);
	avCodec.detach();
	#endif

	// Led Stuff
	#if RUN_LEDS
	Apa102 lights(10, 29);
	lights.init(0, SPI_BAUD, 0);
	
	int64_t lastLedUpdateTime = lastPrintTime;
	bool on = true;
	bool goUp = true;
	uint32_t i = 0;
	uint32_t j = 0;
	Pixel pixel = { 1, 0xFF, 0x00, 0x00 };
	Pixel nextPixel = { 1, 0x00, 0xFF, 0x00 };
	lights.clear();
	#endif

	int frameNum = 0;

	while (true) {
		int64_t currentTime = Clock::instance().millis();

		#if TERMINATE_PROGRAM
		if (currentTime >= RUN_INTERVAL) {
			break;
		}
		#endif

		if (lastPrintTime + PRINT_INTERVAL <= currentTime) {
			cout << "Current program time in milliseconds: " << currentTime << endl;
			lastPrintTime = currentTime;
		}

		#if RUN_SERIAL_NETWORKING
		if (last_serial_publish + PRINT_INTERVAL <= currentTime) {
			char buf[32] = "Hello World";
			i2cWriteBlockData(handle, 13, buf, 32);
			last_serial_publish = currentTime;

			int messageLength;
			if ((messageLength = i2cReadBlockData(handle, SERIAL_ID, buf)) > 0) {
				printf("%d byte message read from I2C: %c", messageLength, buf);
			}
		}
		#endif

		#if RUN_BRIGHTNESS_TEST
		if (lastLedUpdateTime + LED_INTERVAL <= currentTime) {
			lights.fillArea(Pixel { 31, (uint8_t)i, 0, (uint8_t)i }, Point { 0, 5 }, Point { 9, 15 });
			lights.show();
			if (goUp) {
				i = (i + 1) % 256;
			} else {
				i = (i - 1) % 256;
			}
			
			if (i == 5) {
				goUp = true;
			}
			if (i == 100) {
				goUp = false;
			}

			j = i / 8;
			lastLedUpdateTime = currentTime;
		}
		#endif

		#if RUN_FILL_TEST
		if (lastLedUpdateTime + LED_INTERVAL <= currentTime) {
			if (on) {
				lights.clear();
				if (j > i) {
					lights.fillArea(pixel, Point { j, 0 }, Point { lights.getActiveLeds() - 1, 0 });
					lights.fillArea(nextPixel, Point { 0, 0 }, Point { i, 0 });
				} else {
					lights.fillArea(pixel, Point { i, 0 }, Point { j, 0 });
				}
				// lights.setPixel(Pixel { 31, 0xFF, 0x0, 0x0 }, Point { i, 0 });
				i++; j++;
			}
			lights.show();

			i = i % lights.getActiveLeds();
			j = j % lights.getActiveLeds();
			// on = !on;
			lastLedUpdateTime = currentTime;

			if (j == 0) {
				pixel = nextPixel;
				if (pixel.r) {
					nextPixel = { 1, 0x00, 0xFF, 0x00 };
				} else if (pixel.g) {
					nextPixel = { 1, 0x00, 0x00, 0xFF };
				} else {
					nextPixel = { 1, 0xFF, 0x00, 0x00 };
				}
			}
		}
		#endif
	}

	return 1;
}
