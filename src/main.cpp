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

#define RUN_TIME_MILLIS 45000
#define VIDEO_CHANGE_INTERVAL 10000

const std::vector<std::string> projectFolders = { CARD_VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY, WAIT_VIDEO_DIRECTORY };
const std::vector<std::string> videoFolders1 = { CARD_VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY };
const std::vector<std::string> videoFolders2 = { WAIT_VIDEO_DIRECTORY };

VideoPlayer player1(videoFolders1);
VideoPlayer player2(videoFolders2);

const std::vector<VideoPlayer*> players = { &player1, &player2 };

const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };

int main(int argc, char** argv) {
	system("sudo sh -c \"TERM=linux setterm -foreground black -clear all >/dev/tty0\"");

	// Init
	int64_t startTime = Clock::instance().millis();
	DataManager* manager = new DataManager();
	manager->init();
	
	for (VideoPlayer* player : players) {
		player->init(VLC_ARGS, 9);
		manager->addMediaListener(player);
	}
	int64_t endTime = Clock::instance().millis();

	std::cout << "Reading from folders took " << endTime - startTime << "ms" << std::endl;

	int64_t sum = 0;
	int64_t count = 0;
	int64_t lastTime = -VIDEO_CHANGE_INTERVAL;

	lastTime = Clock::instance().millis();
	const auto& vec = manager->getFileIdsFromFolder(CARD_VIDEO_DIRECTORY);

	player1.setCurrentMedia(vec[0]);
	player1.playLoop();

	while (true) {
		if (Clock::instance().millis() > RUN_TIME_MILLIS) {
			break;
		}

		manager->run();

		// if (Clock::instance().millis() > lastTime + VIDEO_CHANGE_INTERVAL) {
		// 	lastTime = Clock::instance().millis();
		// 	const auto& vec = manager->getFileIdsFromFolder(CARD_VIDEO_DIRECTORY);

		// 	std::default_random_engine generator;
		// 	std::uniform_int_distribution<int32_t> distribution(0, vec.size() - 1);
		// 	int32_t i = distribution(generator);
		// 	player1.setCurrentMedia(vec[i]);
		// 	player1.playLoop();
		// 	std::cout << "Playing media at index: " << i << std::endl;
		// }

		sum += (endTime - startTime);
		count++;
	}

	if (count > 0)
		std::cout << "Avg runtime for DataManager::updateFilesFromFolders: " << sum / count << " micro seconds" << std::endl;

	delete manager;
	system("sudo sh -c \"TERM=linux setterm -foreground white >/dev/tty0\"");
	return 1;
}