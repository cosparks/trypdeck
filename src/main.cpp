#include <iostream>
#include <thread>
#include <pigpio.h>
#include <vlc/vlc.h>
#include <stdlib.h>
#include <ws2811/ws2811.h>

#include "Clock.h"

using namespace std;

#define RUN_INTERVAL 30000
#define PRINT_INTERVAL 1000

bool initializeGpio() {
	if (gpioInitialise() < 0) {
		cout << "PI GPIO Initialization failed" << endl;
		return false;
	}
	else {
		cout << "PI GPIO Initialization successful" << endl;
		return true;
	}
}

void play_video() {
	system("omxplayer /home/trypdeck/projects/tripdeck_basscoast/src/video/climbing.m4v");
}

void initializeVlc() {
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	const char* args[] = { "-v", "-I", "dummy", "--fullscreen", "--no-osd", "--x11-display", ":0" };
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

	/* Let it play a bit */
	this_thread::sleep_for(chrono::seconds(40));

	/* Stop playing */
	libvlc_media_player_stop(mp);

	/* Free the media_player */
	libvlc_media_player_release(mp);

	libvlc_release(inst);
}

void initializeWS281X() {
	ws2811_t ledString =
	{
		.freq = WS2811_TARGET_FREQ,
		.dmanum = 10,
		.channel =
		{
			[0] =
			{
				.gpionum = 18,
				.invert = 0,
				.count = 100,
				.strip_type = WS2811_STRIP_GBR,
				.brightness = 255,
			},
			[1] =
			{
				.gpionum = 0,
				.invert = 0,
				.count = 0,
				.brightness = 0,
			},
		},
	};

	ws2811_return_t ret;

	if ((ret = ws2811_init(&ledString)) != WS2811_SUCCESS) {
		cout << "failed to initialize ws2811.  Error: " << ret << endl;
	}
	ws2811_led_t* matrix = (ws2811_led_t*)malloc(sizeof(ws2811_led_t) * 100);
}

int main(int argv, char** argc) {
	//if (!initializeGpio()) {
	//	return -1;
	//}

	//initializeVlc();

	thread omx(play_video);
	omx.join();
	
	int64_t lastTime = Clock::instance().millis();

	while (true) {
		int64_t currentTime = Clock::instance().millis();

		if (currentTime >= RUN_INTERVAL) {
			break;
		}

		if (lastTime + PRINT_INTERVAL <= currentTime) {
			cout << "Current running time in millis is: " << currentTime << endl;
			lastTime = currentTime;
		}
	}
	return 1;
}
