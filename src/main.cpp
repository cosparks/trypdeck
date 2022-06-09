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

#define RUN_TIME_MILLIS 330000000
#define VIDEO_PLAY_INTERVAL 10000
#define VLC_DELAY 600

const std::vector<std::string> projectFolders = { CARD_VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY, WAIT_VIDEO_DIRECTORY };
const std::vector<std::string> videoFolders1 = { CARD_VIDEO_DIRECTORY };
const std::vector<std::string> videoFolders2 = { WAIT_VIDEO_DIRECTORY };
const std::vector<std::string> ledFolders = { LED_ANIMATION_DIRECTORY };

int main(int argc, char** argv) {
	Apa102 apa102(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_GRID_CONFIGURATION_OPTION);
	VideoPlayer videoPlayer1(videoFolders1);
	VideoPlayer videoPlayer2(videoFolders2);
	LedPlayer ledPlayer(ledFolders, &apa102);
	DataManager manager;

	const std::vector<MediaPlayer*> players = { &videoPlayer1, &videoPlayer2, &ledPlayer };

	int64_t startTime = Clock::instance().millis();
	// clear screen
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// initialize objects 
	manager.init();
	for (MediaPlayer* player : players) {
		// for each media player, initialize and hook it up to its respective media folders via DataManager
		player->init();
		manager.addMediaListener(player);
	}
	int64_t endTime = Clock::instance().millis();
	std::cout << "Initializing DataManager and MediaPlayers took " << endTime - startTime << "ms" << std::endl;

	// TEST CODE
	const auto& vec1 = manager.getFileIdsFromFolder(LED_ANIMATION_DIRECTORY);
	const auto& vec2 = manager.getFileIdsFromFolder(CARD_VIDEO_DIRECTORY);

	ledPlayer.setCurrentMedia(vec1[0], MediaPlayer::Loop);
	ledPlayer.play();
	videoPlayer1.setCurrentMedia(vec2[0], MediaPlayer::Loop);
	videoPlayer1.play();

	startTime = Clock::instance().millis();
	while (true) {
		int64_t currentTime = Clock::instance().millis();

		if (currentTime > RUN_TIME_MILLIS) {
			break;
		}

		manager.run();
		ledPlayer.run();

		// TEST LED / VIDEO TIMING
		// if (currentTime > VIDEO_PLAY_INTERVAL + startTime) {
		// 	ledPlayer.stop();
		// 	videoPlayer1.stop();

		// 	videoPlayer1.play();
		// 	ledPlayer.play();
		// 	startTime = currentTime;
		// }
	}

	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}