#include <iostream>
#include <thread>
#include <pigpio.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_media.h>


#include "Clock.h"

using namespace std;

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

void initializeVlc() {
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	/* Load the VLC engine */
	inst = libvlc_new (0, NULL);
	/* Create a new item */
	m = libvlc_media_new_location (inst, "http://mycool.movie.com/test.mov");
	//m = libvlc_media_new_path (inst, "/path/to/test.mov");
	/* Create a media player playing environement */
	mp = libvlc_media_player_new_from_media (m);

	/* No need to keep the media now */
	libvlc_media_release (m);
	/* play the media_player */
	libvlc_media_player_play (mp);

	/* Let it play a bit */
	this_thread::sleep_for(chrono::seconds(1));

	/* Stop playing */
	libvlc_media_player_stop (mp);
	/* Free the media_player */
	libvlc_media_player_release (mp);
	libvlc_release (inst);
}

int main(int argv, char** argc) {
	if (!initializeGpio()) {
		return -1;
	}

	initializeVlc();
	
	bool run = true;
	int64_t lastTime = Clock::instance().millis();
	while (run) {
		if (Clock::instance().millis() >= lastTime + PRINT_INTERVAL) {
			cout << "The current time in milliseconds is: " << Clock::instance().millis() << endl;
			cout << "The current time in seconds is: " << Clock::instance().seconds() << endl;
			cout << "Yayy" << endl;
			lastTime = Clock::instance().millis();
		}

	}
}
