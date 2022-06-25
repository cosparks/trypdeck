#include <iostream>

#include "settings.h"
#include "Clock.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"
#include "Tripdeck.h"

const char* VideoFolders[] = { VIDEO_STARTUP_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_STARTUP_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

int main(int argc, char** argv) {
	int64_t startTime = Clock::instance().millis();
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");
	
	// construct main objects
	#ifdef LED_GRID_CONFIGURATION_OPTION_A
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, LED_GRID_AB_ORIENTATION,
		LED_GRID_CONFIGURATION_OPTION_A, LED_GRID_CONFIGURATION_OPTION_B);
	#else
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 0, LED_GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION,
		Apa102::GridConfigurationOption(0));
	#endif
	LedPlayer ledPlayer(&ledController);
	VideoPlayer videoPlayer;
	DataManager dataManager;
	Tripdeck application(&dataManager, &ledPlayer, &videoPlayer);

	// add media folders for application states
	for (int32_t state = Tripdeck::TripdeckState::Startup; state <= Tripdeck::TripdeckState::Reveal; state++) {
		application.addLedFolder(Tripdeck::TripdeckState(state), LedFolders[state]);
		application.addVideoFolder(Tripdeck::TripdeckState(state), VideoFolders[state]);
	}

	// initialize and run application
	application.init();

	int64_t endTime = Clock::instance().millis();
	std::cout << "Initializing application took " << endTime - startTime << "ms" << std::endl;

	application.run();

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}