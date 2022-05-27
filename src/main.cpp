#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "settings.h"
#include "Clock.h"
#include "DataManager.h"
#include "Index.h"
#include "VideoPlayer.h"

#define RUN_TIME_MILLIS 15000

const std::vector<std::string> projectFolders = { CARD_VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY, WAIT_VIDEO_DIRECTORY };

const std::vector<std::string> videoFolders = { CARD_VIDEO_DIRECTORY, WAIT_VIDEO_DIRECTORY };
const char* VLC_ARGS[] = { "-v", "-I", "dummy", "--aout=adummy", "--fullscreen", "--no-osd", "--no-audio", "--vout", "mmal_vout" };

int main(int argc, char** argv) {
	DataManager* manager = new DataManager(projectFolders);
	VideoPlayer* player = new VideoPlayer(videoFolders);
	// player->init(manager->getFileIdsFromFolder(CARD_VIDEO_DIRECTORY), VLC_ARGS, std::extent<decltype(VLC_ARGS)>::value);

	int64_t startTime = Clock::instance().millis();
	manager->init();

	int64_t endTime = Clock::instance().millis();

	std::cout << "Reading from folder took " << endTime - startTime << "ms" << std::endl;
	std::cout << "card: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[0])) {
		std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}
	std::cout << "anim: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[1])) {
			std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}
	std::cout << "loop: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[2])) {
		std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}

	int64_t sum = 0;
	int64_t count = 0;
	while (true) {
		if (Clock::instance().millis() > RUN_TIME_MILLIS) {
			break;
		}
		startTime = Clock::instance().micros();
		manager->run();
		endTime = Clock::instance().micros();

		sum += (endTime - startTime);
		count++;
	}

	std::cout << "card: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[0])) {
		std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}
	std::cout << "anim: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[1])) {
		std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}
	std::cout << "loop: " << std::endl;
	for (uint32_t path : manager->getFileIdsFromFolder(projectFolders[2])) {
		std::cout << "\t" << path << "\t -- full path: " << Index::instance().getSystemPath(path) << std::endl;
	}

	if (count > 0)
		std::cout << "Avg runtime for DataManager::updateFilesFromFolders: " << sum / count << " micro seconds" << std::endl;

	delete manager;
	return 1;
}