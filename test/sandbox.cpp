#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <pigpio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <stack>
#include <functional>

#include <sys/types.h>
#include <pwd.h>

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

#include <queue>

#include "settings.h"
#include "Serial.h"
#include "Apa102.h"
#include "Clock.h"

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

using namespace std;

#define SERIAL_BAUD_TEST 9600
#define SPI_BAUD_TEST 6000000

// PROGRAM MACROS
#define DEBUG_MODE 0

#define RUN_AV_DECODING 0
#define PLAY_RGB_FRAMES 0
#define RUN_LED_TEST PLAY_RGB_FRAMES
#define PLAY_FRAMES_CORRECT_TIMING 1
#define RUN_DECODE_PERFORMANCE_TESTING 0
#define TRANSCODE_VIDEO_PATH "/home/trypdeck/projects/tripdeck_basscoast/media/loop/nyan-cat.mp4"

#define PLAY_OMX 0
#define STOP_OMX_ARGS "killall omxplayer.bin" // MAYBE ADD "omxplayer -p -o hdmi /home/trypdeck/projects/tripdeck_basscoast/media/loop/elden.mp4"
#define OMX_ARGS1 "omxplayer --no-osd /home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/elden.mp4"
#define OMX_ARGS2 "omxplayer --loop --no-osd /home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/elden.mp4"

#define PLAY_VLC 0
#define PRINT_USER_INFO 0
#define RUN_SERIAL_NETWORKING 0
#define START_VIDEO_VLC "nyan-cat.mp4"

#define RUN_MULTITHREADING 0
#define TERMINATE_PROGRAM 1
#define PROGRAM_RUN_TIME 50000
#define MULTI_PURPOSE_INTERVAL 5000

// GPIO
#define RUN_GPIO_TEST 1
#define GPIO_READ_PIN_1 23
#define GPIO_READ_PIN_2 24
#define DEBOUNCE_MILLIS 75

// LED
#define PIXEL_BRIGHTNESS_TEST 31
#define MATRIX_WIDTH 53
#define MATRIX_HEIGHT 10

#define INBUF_SIZE 4096

Apa102 lights(MATRIX_WIDTH, MATRIX_HEIGHT, Apa102::HorizontalTopLeft);

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

void on_thread_exit(std::function<void()> func) {
	class ThreadExiter {
		public:
			ThreadExiter() = default;
			ThreadExiter(ThreadExiter const&) = delete;
			void operator=(ThreadExiter const&) = delete;
			~ThreadExiter() {
				while (!exit_funcs.empty()) {
					exit_funcs.top()();
					exit_funcs.pop();
				}
			}
			void add(std::function<void()> func) {
				exit_funcs.push(std::move(func));
			}
		private:
			std::stack<std::function<void()>> exit_funcs;
	};

	thread_local ThreadExiter exiter;
	exiter.add(std::move(func));
}

void stop_omx() {
	system(STOP_OMX_ARGS);
}

void play_video_omx(const char* omxArgs, std::function<void()> func) {
	on_thread_exit(func);
	system(omxArgs);
}

void play_start_video() {
	play_video_omx(OMX_ARGS2, [](void) -> void { std::cout << "Thread terminated" << std::endl; });
}

static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static const char *src_filename = NULL;
static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static AVRational stream_time_base;

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
		fprintf(stderr, "Could not find %s stream in input file '%s'\n", media_type_string(type), src_filename);
		return ret;
	}
	else {
		*stream_idx = ret;
		st = fmt_ctx->streams[*stream_idx];
		stream_time_base = st->time_base;
		// find decoder for the stream
		dec_param = st->codecpar;
		dec = avcodec_find_decoder(dec_param->codec_id);
		if (!dec) {
		fprintf(stderr, "Failed to find %s codec\n", media_type_string(type));
			return ret;
		}
		video_dec_ctx = avcodec_alloc_context3(dec);
		avcodec_parameters_to_context(video_dec_ctx, dec_param);
		if ((ret = avcodec_open2(video_dec_ctx, dec, NULL)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n", media_type_string(type));
			return ret;
		}
	}
	return 0;
}

void playFrame(AVFrame* frameRGB) {
	int32_t pixelsPerLedX = frameRGB->width / MATRIX_WIDTH;
	int32_t pixelsPerLedY = frameRGB->height / MATRIX_HEIGHT;
	// uint32_t offsetX = pixelsPerLedX / 2;
	// uint32_t offsetY = pixelsPerLedY / 2;
	int32_t offsetX = 0;
	int32_t offsetY = 0;


	#if RUN_LED_TEST
	lights.clear();
	#endif

	// Write pixel data
	for(int32_t y = 0; y < MATRIX_HEIGHT; y++) {
		for (int32_t x = 0; x < MATRIX_WIDTH; x++) {
			uint8_t* ptr = frameRGB->data[0] + (y * pixelsPerLedY + offsetY) * frameRGB->linesize[0] + 3 * (x * pixelsPerLedX + offsetX);
			#if RUN_LED_TEST
			lights.setPixel(Pixel { PIXEL_BRIGHTNESS_TEST, ptr[0], ptr[1], ptr[2] }, Point { x, y });
			#endif
		}
	}
	#if RUN_LED_TEST
	lights.show();
	#endif
}

int av_read_frame_log_time(AVFormatContext* context, AVPacket* packet, queue<int64_t>& queue) {
	queue.emplace(Clock::instance().micros());
	return av_read_frame(context, packet);
}

static void video_decode_example()
{
	AVPacket avpkt;
	av_init_packet(&avpkt);

	int ret = 0;
	frame = 0;
	SwsContext* swsContext = sws_getContext(video_dec_ctx->width, video_dec_ctx->height,
		AVPixelFormat(video_dec_ctx->pix_fmt), video_dec_ctx->width, video_dec_ctx->height,
		AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

	AVFrame *frame = av_frame_alloc();
	AVFrame* frameRGB = av_frame_alloc(); frameRGB->format = AV_PIX_FMT_RGB24; frameRGB->width = video_dec_ctx->width; frameRGB->height = video_dec_ctx->height;
	if (av_frame_get_buffer(frameRGB, 32) < 0) {
		std::cout << "Problem allocating AVFrame for RGB values" << std::endl;
	}

	std::cout << "Stream time base: " << stream_time_base.num << " / " << stream_time_base.den << std::endl;

	// PERFORMANCE Testing
	std::queue<int64_t> performanceQueue;

	#if RUN_DECODE_PERFORMANCE_TESTING
	int64_t start = Clock::instance().seconds();
	int fpsCount = 0;
	#endif

	#if PLAY_FRAMES_CORRECT_TIMING
	int i = 0;
	int startTime = Clock::instance().micros();
	#endif

	// Read all the frames
	while (av_read_frame_log_time(fmt_ctx, &avpkt, performanceQueue) >= 0) {
		if (avpkt.size == 0)
			break;
		// fprintf(stderr, "Video frame size %d\r\n", avpkt.size);
		// cout << "Program time after read frame: " << Clock::instance().millis() << endl;

		// This function might fail because of parameter set packets, just ignore and continue
		ret = avcodec_send_packet(video_dec_ctx, &avpkt);
		if (ret < 0) {
			performanceQueue.pop();
			fprintf(stderr, "avcodec_send_packet ret < 0\n");
			continue;
		}

		//cout << "Program time before avcodec receive frame: " << Clock::instance().millis() << endl;
		// Receive the uncompressed frame back
		ret = avcodec_receive_frame(video_dec_ctx, frame);
		
		//cout << "Program time before frame converted to RGB: " << Clock::instance().millis() << endl;

		if (ret < 0) {
			// Sometimes we cannot get a new frame, continue in this case
			if (ret == AVERROR(EAGAIN)) {
				performanceQueue.pop();
				continue;
			}

			fprintf(stderr, "avcodec_receive_frame ret < 0\n");
			break;
		}

		// if (frameTimeEstimate >= 9.99) {
		// 	std::cout << "Estimated fps based on frame time stamps: " << fpsCount / 10 << "fps" << std::endl;
		// 	return;
		// }

		// Convert frame data to RGB
		sws_scale(swsContext, (uint8_t const * const *)frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);

		#if PLAY_RGB_FRAMES
		#if PLAY_FRAMES_CORRECT_TIMING
		double frameTimeEstimate = (double)frame->best_effort_timestamp * ((double)stream_time_base.num / (double)stream_time_base.den);
		std::cout << "frame time stamp offset: " << frame->best_effort_timestamp << endl;
		std::cout << "actual time: " << frameTimeEstimate << endl;
		int64_t nextFrameTime = startTime + frame->best_effort_timestamp * (1000000L / stream_time_base.den);
		int i = 0;
		while (nextFrameTime > Clock::instance().micros()) {
			i++;
		}
		#endif
		playFrame(frameRGB);
		#endif

		#if RUN_DECODE_PERFORMANCE_TESTING
		performanceQueue.emplace(Clock::instance().micros());
		fpsCount++;
		#endif
	}

	#if RUN_DECODE_PERFORMANCE_TESTING
	// TOTAL AND AVERAGE FRAME TIMES
	int64_t end = Clock::instance().seconds();
	performanceQueue.emplace(0x0);
	int64_t sum = 0;
	int64_t count = 0;
	while (!performanceQueue.empty()) {
		int64_t preDecodeTime = performanceQueue.front();
		performanceQueue.pop();
		int64_t postDecodeTime = performanceQueue.front();
		performanceQueue.pop();

		if (postDecodeTime == 0) {
			break;
		}
		sum += (postDecodeTime - preDecodeTime);
		count++;
	}

	cout << "Total Transcoding time in seconds is: " << end - start << endl;
	cout << "The average time to read a frame in microseconds is: " << sum / count << endl;
	cout << "The average time to read a frame in milliseconds is: " << (sum / count) / 1000 << endl;
	cout << "Average frames per second: " << 1000 / ((sum / count) / 1000) << endl;
	#endif

	fprintf(stderr, "out of loop av_read_frame\n");
	avcodec_close(video_dec_ctx);
	av_free(video_dec_ctx);
	sws_freeContext(swsContext);
	av_frame_free(&frame);
	av_frame_free(&frameRGB);
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

	#if RUN_LED_TEST || RUN_GPIO_TEST
	if (!initializeGpio()) {
		return -1;
	}

	lights.init(0, SPI_BAUD_TEST, 0);
	lights.clear();
	cout << "Apa102 initialized with: " << lights.getNumLeds() << " leds" << endl;
	#endif

	#if RUN_SERIAL_NETWORKING
	Serial serial("/dev/ttyS0", O_RDWR);
	serial.init();
	int64_t last_serial_publish = Clock::instance().millis();
	#endif

	#if PLAY_OMX
	bool first = true;
	uint32_t threadCount = 1;
	std::thread(play_video_omx, OMX_ARGS1, [threadCount](void) -> void { std::cout << "Thread number " << threadCount << " has exited" << std::endl; }).detach();
	#endif

	#if RUN_AV_DECODING
	#if RUN_MULTITHREADING
	thread avCodec(runAv);
	avCodec.detach();
	#else
	runAv();
	#endif
	#endif

	#if RUN_GPIO_TEST
	int32_t ret = gpioSetMode(GPIO_READ_PIN_1, PI_INPUT);
	if (ret != 0) {
		throw std::runtime_error("Error code " + std::to_string(ret) + ": failed to setup pin 24 as input");
	}

	ret = gpioSetMode(GPIO_READ_PIN_2, PI_INPUT);
	if (ret != 0) {
		throw std::runtime_error("Error code " + std::to_string(ret) + ": failed to setup pin 24 as input");
	}

	int32_t lastReadValue1 = 0;
	int64_t lastReadTime1 = 0;
	int32_t lastReadValue2 = 0;
	int64_t lastReadTime2 = 0;
	#endif


	while (true) {
		int64_t currentTime = Clock::instance().millis();

		#if TERMINATE_PROGRAM
		if (currentTime >= PROGRAM_RUN_TIME) {
			break;
		}
		#endif

		#if RUN_GPIO_TEST
		
		int64_t before = Clock::instance().micros();
		int32_t ret = gpioRead(GPIO_READ_PIN_1);
		int64_t after = Clock::instance().micros();

		if (lastReadValue1 != ret && currentTime > lastReadTime1 + DEBOUNCE_MILLIS) {

			if (ret == PI_BAD_GPIO) {
				std::cout << "Received PI_BAD_GPIO on read from GPIO_READ_PIN_1" << std::endl;
			} else {
				std::cout << "Read " << ret << " from GPIO_READ_PIN_1 in " << after - before << " micros" << std::endl;
			}

			lastReadValue1 = ret;
			lastReadTime1 = currentTime;
		}

		before = Clock::instance().micros();
		ret = gpioRead(GPIO_READ_PIN_2);
		after = Clock::instance().micros();

		if (lastReadValue2 != ret && currentTime > lastReadTime2 + DEBOUNCE_MILLIS) {

			if (ret == PI_BAD_GPIO) {
				std::cout << "Received PI_BAD_GPIO on read from GPIO_READ_PIN_2" << std::endl;
			} else {
				std::cout << "Read " << ret << " from GPIO_READ_PIN_2 in " << after - before << " micros" << std::endl;
			}

			lastReadValue2 = ret;
			lastReadTime2 = currentTime;
		}

		#endif
		
		#if PLAY_OMX
		if (currentTime > lastPrintTime + MULTI_PURPOSE_INTERVAL) {
			stop_omx();
			threadCount++;
			if (first)
				std::thread(play_video_omx, OMX_ARGS1, [threadCount](void) -> void { std::cout << "Thread number " << threadCount << " has exited" << std::endl; }).detach();
			else
				std::thread(play_video_omx, OMX_ARGS2, [threadCount](void) -> void { std::cout << "Thread number " << threadCount << " has exited" << std::endl; }).detach();

			first = !first;
			lastPrintTime = currentTime;
		}
		#endif
	}

	return 1;
}
