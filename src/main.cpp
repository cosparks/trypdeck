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
#include <ws2811/ws2811.h>

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

using namespace std;

#define SPI_BAUD 6000000

#define RUN_INTERVAL 80000
#define PRINT_INTERVAL 1000
#define LED_INTERVAL 50
#define MATRIX_WIDTH 60
#define MATRIX_HEIGHT 40

#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif


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
	system("omxplayer /home/trypdeck/projects/tripdeck_basscoast/src/video/climbing.mp4");
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
	m = libvlc_media_new_path (inst, "video/climbing.m4v");
	
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
	// this_thread::sleep_for(chrono::seconds(40));
}

void runVlc() {
	// /* Stop playing */
	// libvlc_media_player_stop(mp);

	// /* Free the media_player */
	// libvlc_media_player_release(mp);

	// libvlc_release(inst);
}

void runAddressableLeds(int64_t currentTime, int64_t& lastTime, uint32_t& i, uint32_t& j, bool& on) {
	if (lastTime + LED_INTERVAL <= currentTime) {
		if (on) {
			// writeSingleLed(j, 31, 0xFF, 0, 0);
			// i++;
			// APA102_Fill(strip, APA102_CreateFrame(31, 0xFF, 0x0, 0x00));
		} else {
			// writeSingleLed(j, 0, 0, 0, 0);
			// APA102_Fill(strip, APA102_CreateFrame(0, 0x0, 0x0, 0x0));
		}

		// APA102_Clear();
		// APA102_WriteLEDSegment(i, j, APA102_CreateFrame(31, 0xFF, 0x0, 0x0));
		// i = (i + 1) % ACTIVE_LEDS;
		// j = (j + 1) % ACTIVE_LEDS;
		// writeSingleLed(j, 31, 0xFF, 0x00, 0);

		// writeSingleLed(i++ % NUM_LEDS, 31, 0xFF, 0xFF, 0xFF);
		lastTime = currentTime;

		// Remove this
		// j = (j - 1);
		// if (j == 0) {
		// 	j = 300;
		// }

		on = !on;
		// if (i > 3) {
		// 	i = 0;
		// 	// Uncomment this
		// 	j = (j + 1) % ACTIVE_LEDS;
		// 	if (j == 0) {
		// 		APA102_Clear();
		// 	}
		// }
	}
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

		iFrame++;
		if (!(iFrame % 4 == 0)) {
			continue;
		}

		// Convert frame data to RGB
		SwsContext* swsContext = sws_getContext(frame->width, frame->height, AVPixelFormat(frame->format), frame->width, frame->height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		AVFrame* frameRGB = av_frame_alloc(); frameRGB->format = AV_PIX_FMT_RGB24; frameRGB->width = frame->width; frameRGB->height = frame->height;
		av_frame_get_buffer(frameRGB, 32);
		sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, frameRGB->data, frameRGB->linesize);
		sws_freeContext(swsContext);
		
		saveFrame(frameRGB, iFrame);
		
		//cout << "Program time after frame converted to RGB: " << Clock::instance().millis() << endl;

		// Calculate output buffer requirements
		// int image_buffer_size = av_image_get_buffer_size(AVPixelFormat(frameRGB->format), frameRGB->width, frameRGB->height, 1);

		// Print frame info
		// fprintf(stderr,
		// 	"[%d] Got frame of size: %dx%d (%d bytes)\r\n",
		// 	video_dec_ctx->frame_number,
		// 	frame->width,
		// 	frame->height,
		// 	image_buffer_size
		// );

		// Use temp buffer for the video data
		// if (imagebuffer == NULL) imagebuffer = new uint8_t[image_buffer_size];
		// 	av_image_copy_to_buffer(imagebuffer, image_buffer_size, frame->data, frame->linesize, AVPixelFormat(frame->format), frame->width, frame->height, 1);    

		// Dump the frame to a file
		// FILE* out = fopen("out_864x486.yuv", "ab");
		// fwrite(imagebuffer, image_buffer_size, 1, out);
		// fclose(out);
		av_frame_free(&frameRGB);
		i++;
		microBuffer[i++] = Clock::instance().micros();
	}

	// CALCULATE AVERAGE TIMES
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

	cout << "The average time to read a frame in microseconds is: " << sum / count << endl;
	cout << "The average time to read a frame in milliseconds is: " << (sum / count) / 1000 << endl;
	cout << "Frames per second: " << 1000 / ((sum / count) / 1000) << endl;

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
	
	//src_filename = "src/video/test.m4v";
	src_filename = "video/test.m4v";

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
	// if (!initializeGpio()) {
	// 	return -1;
	// }

	// initializeVlc();
	// thread omx(play_video);
	// omx.detach();

	// generate ppm frames
	// thread avCodec(runAv);
	// avCodec.detach();

	// Led Stuff
	// Apa102 lights(10, 29);
	// lights.init(0, SPI_BAUD, 0);
	
	// int64_t lastLedUpdateTime = lastPrintTime;
	// bool on = true;
	// bool goUp = true;
	// uint32_t i = 0;
	// uint32_t j = 0;
	// Pixel pixel = { 1, 0xFF, 0x00, 0x00 };
	// Pixel nextPixel = { 1, 0x00, 0xFF, 0x00 };
	// lights.clear();

	int frameNum = 0;

	while (true) {
		int64_t currentTime = Clock::instance().millis();

		if (currentTime >= RUN_INTERVAL) {
			break;
		}

		if (lastPrintTime + PRINT_INTERVAL <= currentTime) {
			cout << "Current program time in milliseconds: " << currentTime << endl;
			lastPrintTime = currentTime;
		}

		// FILE *pFile;
		// char szFilename[32];

		// // Open file
		fstream led_stream;
		char fileName[32];
		sprintf(fileName, "src/gen/frame%d.ppm", frameNum+=2);
		led_stream.open(fileName, ios::out);

		if (!led_stream.is_open()) {
			return -1;
		}

		char header[32];
		led_stream.read(header, 32);

		char data[32];
		for (int i = 0; i < 32; i++) {
			data[i] = header[i];
		}

		printf(data);
		
		// sprintf(szFilename, "src/gen/frame%d.ppm", frameNum+=2);
		// pFile = fopen(szFilename, "wb");
		// pFile;
		// Brightness Test
		// Purple #A020F0
		// if (lastLedUpdateTime + LED_INTERVAL <= currentTime) {
		// 	lights.fillArea(Pixel { 31, (uint8_t)i, 0, (uint8_t)i }, Point { 0, 5 }, Point { 9, 15 });
		// 	lights.show();
		// 	if (goUp) {
		// 		i = (i + 1) % 256;
		// 	} else {
		// 		i = (i - 1) % 256;
		// 	}
			
		// 	if (i == 5) {
		// 		goUp = true;
		// 	}
		// 	if (i == 100) {
		// 		goUp = false;
		// 	}

		// 	j = i / 8;
		// 	lastLedUpdateTime = currentTime;
		// }

		// Fill Test
		// if (lastLedUpdateTime + LED_INTERVAL <= currentTime) {
		// 	if (on) {
		// 		lights.clear();
		// 		if (j > i) {
		// 			lights.fillArea(pixel, Point { j, 0 }, Point { lights.getActiveLeds() - 1, 0 });
		// 			lights.fillArea(nextPixel, Point { 0, 0 }, Point { i, 0 });
		// 		} else {
		// 			lights.fillArea(pixel, Point { i, 0 }, Point { j, 0 });
		// 		}
		// 		// lights.setPixel(Pixel { 31, 0xFF, 0x0, 0x0 }, Point { i, 0 });
		// 		i++; j++;
		// 	}
		// 	lights.show();

		// 	i = i % lights.getActiveLeds();
		// 	j = j % lights.getActiveLeds();
		// 	// on = !on;
		// 	lastLedUpdateTime = currentTime;

		// 	if (j == 0) {
		// 		pixel = nextPixel;
		// 		if (pixel.r) {
		// 			nextPixel = { 1, 0x00, 0xFF, 0x00 };
		// 		} else if (pixel.g) {
		// 			nextPixel = { 1, 0x00, 0x00, 0xFF };
		// 		} else {
		// 			nextPixel = { 1, 0xFF, 0x00, 0x00 };
		// 		}
		// 	}
		// }
	}


	return 1;
}
