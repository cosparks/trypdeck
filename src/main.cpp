#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include <random>

#include "settings.h"
#include "Clock.h"
#include "DataManager.h"
#include "Index.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"
#include "Tripdeck.h"

#define RUN_TIME_MILLIS 330000000
#define VIDEO_PLAY_INTERVAL 10000
#define VLC_DELAY 600

const char* VideoFolders[] = { VIDEO_STARTUP_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_STARTUP_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

int main(int argc, char** argv) {
	int64_t startTime = Clock::instance().millis();
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	Apa102 apa102(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_GRID_CONFIGURATION_OPTION);
	VideoPlayer videoPlayer;
	LedPlayer ledPlayer(&apa102);
	DataManager dataManager;
	Tripdeck application(&dataManager, &ledPlayer, &videoPlayer);

	for (int32_t state = Tripdeck::TripdeckState::Startup; state <= Tripdeck::TripdeckState::Startup; state++) {
		application.addLedFolder(Tripdeck::TripdeckState(state), VideoFolders[state]);
		application.addVideoFolder(Tripdeck::TripdeckState(state), LedFolders[state]);
	}

	application.init();
	int64_t endTime = Clock::instance().millis();
	std::cout << "Initializing application took " << endTime - startTime << "ms" << std::endl;

	application.run();
	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}