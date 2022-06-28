#include <iostream>

#include "settings.h"
#include "Clock.h"
#include "Serial.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"
#include "TripdeckMediaManager.h"
#include "TripdeckLeader.h"
#include "TripdeckFollower.h"
#include "MockButton.h"

const char* VideoFolders[] = { VIDEO_CONNECTING_DIRECTORY, VIDEO_CONNECTED_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_CONNECTING_DIRECTORY, LED_CONNECTED_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

InputManager inputManager;
Serial serial("/dev/ttyAMA0", O_RDWR);

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// set up media
	DataManager dataManager;
	VideoPlayer videoPlayer;
	#if RUN_LEDS
	#if (LED_SETTING == MAIN_LEDS)
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, LED_CONTROLLER_ORIENTATION, LED_GRID_CONFIGURATION_OPTION_A, LED_GRID_CONFIGURATION_OPTION_B);
	#else
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 0, LED_CONTROLLER_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, Apa102::GridConfigurationOption(0));
	#endif
	LedPlayer ledPlayer(&ledController);
	TripdeckMediaManager mediaManager(&dataManager, &videoPlayer, &ledPlayer);
	#else
	TripdeckMediaManager mediaManager(&dataManager, &videoPlayer);
	#endif

	for (int32_t state = TripdeckState::Connecting; state <= TripdeckState::Reveal; state++) {
		mediaManager.addVideoFolder(TripdeckState(state), VideoFolders[state]);

		#if RUN_LEDS
		mediaManager.addLedFolder(TripdeckState(state), LedFolders[state]);
		#endif
	}

	// set up application
	#ifdef Leader
	TripdeckLeader tripdeck(&mediaManager, &inputManager, &serial);
	#else
	TripdeckFollower tripdeck(&mediaManager, &inputManager, &serial);
	#endif

	// initialize and run application
	tripdeck.init();
	tripdeck.run();

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}