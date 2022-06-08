#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include <random>

#include "settings.h"
#include "Clock.h"
#include "DataManager.h"
#include "Index.h"
#include "VideoPlayer.h"
#include "LedPlayer.h"

#define RUN_TIME_MILLIS 200000
#define VIDEO_CHANGE_INTERVAL 10000

const std::vector<std::string> projectFolders = { CARD_VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY, WAIT_VIDEO_DIRECTORY };
const std::vector<std::string> videoFolders1 = { CARD_VIDEO_DIRECTORY };
const std::vector<std::string> videoFolders2 = { WAIT_VIDEO_DIRECTORY };
const std::vector<std::string> ledFolders = { LED_ANIMATION_DIRECTORY };

Apa102 apa102(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_GRID_CONFIGURATION_OPTION);
VideoPlayer videoPlayer1(videoFolders1);
VideoPlayer videoPlayer2(videoFolders2);
LedPlayer ledPlayer(ledFolders, &apa102);

const std::vector<MediaPlayer*> players = { &videoPlayer1, &videoPlayer2, &ledPlayer };

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// Init
	int64_t startTime = Clock::instance().millis();
	DataManager* manager = new DataManager();
	manager->init();
	
	for (MediaPlayer* player : players) {
		player->init();
		manager->addMediaListener(player);
	}

	int64_t endTime = Clock::instance().millis();

	std::cout << "Reading from folders took " << endTime - startTime << "ms" << std::endl;

	int64_t sum = 0;
	int64_t count = 0;
	int64_t lastTime = -VIDEO_CHANGE_INTERVAL;

	lastTime = Clock::instance().millis();
	const auto& vec = manager->getFileIdsFromFolder(LED_ANIMATION_DIRECTORY);

	ledPlayer.setCurrentMedia(vec[0], MediaPlayer::MediaPlaybackOption::OneShot);
	ledPlayer.play();

	std::cout << "LedPlayer set to play: " << Index::instance().getSystemPath(vec[0]) << std::endl;

	while (true) {
		if (Clock::instance().millis() > RUN_TIME_MILLIS) {
			break;
		}

		manager->run();
		ledPlayer.run();

		sum += (endTime - startTime);
		count++;
	}

	if (count > 0)
		std::cout << "Avg runtime for DataManager::updateFilesFromFolders: " << sum / count << " micro seconds" << std::endl;

	delete manager;
	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}