#include <iostream>

#include "settings.h"
#include "Clock.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"
#include "Tripdeck.h"
#include "MockButton.h"

const char* VideoFolders[] = { VIDEO_STARTUP_DIRECTORY, VIDEO_WAIT_DIRECTORY, VIDEO_PULLED_DIRECTORY, VIDEO_REVEAL_DIRECTORY };
const char* LedFolders[] = { LED_STARTUP_DIRECTORY, LED_WAIT_DIRECTORY, LED_PULLED_DIRECTORY, LED_REVEAL_DIRECTORY };

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");
	
	DataManager dataManager;
	InputManager inputManager;
	VideoPlayer videoPlayer;

	// set up leds
	#if RUN_LEDS
	#if (LED_SETTING == MAIN_LEDS)
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, LED_CONTROLLER_ORIENTATION, LED_GRID_CONFIGURATION_OPTION_A, LED_GRID_CONFIGURATION_OPTION_B);
	#else
	LedController ledController(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, 0, LED_CONTROLLER_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, Apa102::GridConfigurationOption(0));
	#endif
	LedPlayer ledPlayer(&ledController);
	Tripdeck application(&dataManager, &inputManager, &videoPlayer, &ledPlayer);
	#else
	Tripdeck application(&dataManager, &inputManager, &videoPlayer);
	#endif

	// add media folders for different application states
	for (int32_t state = Tripdeck::TripdeckState::Startup; state <= Tripdeck::TripdeckState::Reveal; state++) {
		application.addVideoFolder(Tripdeck::TripdeckState(state), VideoFolders[state]);

		#if RUN_LEDS
		application.addLedFolder(Tripdeck::TripdeckState(state), LedFolders[state]);
		#endif
	}

	// add inputs for Leader or Follower behavior
	#ifdef Leader
	MockButton input1(0, 5000, 15000);
	MockButton input2(1, 5000, 15000);
	MockButton input3(2, 5000, 15000);
	std::vector<Input*> inputs = { &input1, &input2, &input3 };
	application.addInputs(inputs);
	#else
	
	#endif

	// initialize and run application
	application.init();
	application.run();

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}