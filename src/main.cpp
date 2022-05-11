#include <iostream>
#include <thread>
#include <pigpio.h>
#include <vlc/vlc.h>
#include <stdlib.h>
#include <ws2811/ws2811.h>

#include "Apa102.h"
#include "Clock.h"

using namespace std;

#define RUN_INTERVAL 60000
#define PRINT_INTERVAL 1000
#define LED_INTERVAL 50

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
	m = libvlc_media_new_path (inst, "video/climbing.mp4");
	
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
	// ws2811_led_t* matrix = (ws2811_led_t*)malloc(sizeof(ws2811_led_t) * 100);
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
		i = (i + 1) % ACTIVE_LEDS;
		j = (j + 1) % ACTIVE_LEDS;
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

int main(int argv, char** argc) {
	if (!initializeGpio()) {
		return -1;
	}

	Apa102 lights(300);
	lights.init();

	// initializeVlc();
	// thread omx(play_video);
	// omx.detach();

	int64_t lastPrintTime = Clock::instance().millis();
	int64_t lastLedUpdateTime = lastPrintTime;
	bool on = true;
	uint32_t i = 0;

	while (true) {
		int64_t currentTime = Clock::instance().millis();

		if (currentTime >= RUN_INTERVAL) {
			break;
		}

		if (lastPrintTime + PRINT_INTERVAL <= currentTime) {
			cout << "Current program time in milliseconds: " << currentTime << endl;
			lastPrintTime = currentTime;
		}

		if (lastLedUpdateTime + LED_INTERVAL <= currentTime) {
			if (on) {
				lights.clear();
				lights.setPixel(Pixel { 31, 0xFF, 0x0, 0x0 }, Point { i, 0 });
				i++;
			}
			lights.show();

			i = i % lights.getActiveLeds();
			on = !on;
			lastLedUpdateTime = currentTime;
		}
	}

	return 1;
}
