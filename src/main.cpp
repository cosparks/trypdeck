#include <iostream>
#include <thread>
#include <pigpio.h>
#include <vlc/vlc.h>
#include <stdlib.h>

#include "Clock.h"

using namespace std;

#define RUN_INTERVAL 5000
#define PRINT_INTERVAL 777

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

void initializeVlc() {
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	const char* args = "--quiet --fullscreen --no-osd --x11-display :0 -v";

	/* Load the VLC engine */
	inst = libvlc_new(0, &args);

	/* Create a new item */
	//m = libvlc_media_new_location(inst, "video/climbing.m4v");
	m = libvlc_media_new_path (inst, "video/climbing.mp4");
	
	/* Create a media player playing environement */
	mp = libvlc_media_player_new_from_media(m);

	/* No need to keep the media now */
	libvlc_media_release(m);

	// libvlc_media_player_set_xwindow(mp, xid);

	/* play the media_player */
	libvlc_media_player_play(mp);

	/* Let it play a bit */
	this_thread::sleep_for(chrono::seconds(20));

	/* Stop playing */
	libvlc_media_player_stop(mp);

	/* Free the media_player */
	libvlc_media_player_release(mp);

	libvlc_release(inst);
}

int main(int argv, char** argc) {
	//if (!initializeGpio()) {
	//	return -1;
	//}

	initializeVlc();
	// system("cvlc --quiet --fullscreen --no-osd --x11-display :0 ~/test-files/climbing.m4v");
	
	while (true) {
		if (Clock::instance().millis() >= RUN_INTERVAL) {
			break;
		}
	}
	return 1;
}
