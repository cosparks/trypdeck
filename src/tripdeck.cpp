#include <iostream>
#include <thread>

#include "settings.h"
#include "Clock.h"
#include "Serial.h"
#include "td_pigpio.h"
#include "VideoPlayerVLC.h"
#include "VideoPlayerOmx.h"
#include "LedPlayer.h"
#include "TrypdeckMediaManager.h"
#include "TrypdeckLeader.h"
#include "TrypdeckFollower.h"

const char* VideoFolders[] = { VIDEO_CONNECTING_DIRECTORY, VIDEO_CONNECTED_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_CONNECTING_DIRECTORY, LED_CONNECTED_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// Input and Media
	InputManager inputManager;
	Serial serial("/dev/ttyAMA0", O_RDWR);
	td_pigpio pigpio;
	DataManager dataManager;

	// Video Player
	#if RUN_OMX_PLAYER
	VideoPlayerOmx videoPlayer;
	#else
	VideoPlayerVLC videoPlayer;
	#endif

	// Led Player
	#if RUN_LEDS
	#if (LED_SETTING == MAIN_LEDS)
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, LED_GRID_CONFIGURATION_OPTION_B);
	#else
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 0, GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, Apa102::GridConfigurationOption(0));
	#endif
	LedPlayer ledPlayer(&ledController);
	TrypdeckMediaManager mediaManager(&dataManager, &videoPlayer, &ledPlayer);
	#else
	TrypdeckMediaManager mediaManager(&dataManager, &videoPlayer);
	#endif

	// Add Project Folders
	for (int32_t state = TrypdeckState::Connecting; state <= TrypdeckState::Reveal; state++) {
		mediaManager.addVideoFolder(TrypdeckState(state), VideoFolders[state]);
		mediaManager.addLedFolder(TrypdeckState(state), LedFolders[state]);
	}

	// Application
	#ifdef Leader
	TrypdeckLeader trypdeck(&mediaManager, &inputManager, &serial);
	#else
	TrypdeckFollower trypdeck(&mediaManager, &inputManager, &serial);
	#endif

	// Initialize and Run
	trypdeck.init();
	trypdeck.run();

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}