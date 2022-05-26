#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "settings.h"
#include "Clock.h"
#include "MediaManager.h"

#define RUN_TIME_MILLIS 15000

const std::vector<std::string> paths = { VIDEO_DIRECTORY, LED_ANIMATION_DIRECTORY };

int main(int argc, char** argv) {
	MediaManager* manager = new MediaManager(paths);

	int64_t startTime = Clock::instance().millis();
	manager->init();
	int64_t endTime = Clock::instance().millis();

	std::cout << "Reading from folder took " << endTime - startTime << "ms" << std::endl;
	std::cout << "video: " << std::endl;
	for (const std::string path : manager->getFileUrlsFromFolder(paths[0])) {
		std::cout << "\t" << path << std::endl;
	}
	std::cout << "anim: " << std::endl;
	for (const std::string path : manager->getFileUrlsFromFolder(paths[1])) {
		std::cout << "\t" << path << std::endl;
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

	std::cout << "video: " << std::endl;
	for (const std::string path : manager->getFileUrlsFromFolder(paths[0])) {
		std::cout << "\t" << path << " -- full path: " << manager->getSystemPathFromFileName(path) << std::endl;
	}
	std::cout << "anim: " << std::endl;
	for (const std::string path : manager->getFileUrlsFromFolder(paths[1])) {
		std::cout << "\t" << path << " -- full path: " << manager->getSystemPathFromFileName(path) << std::endl;
	}

	if (count > 0)
		std::cout << "Avg runtime for MediaManager::updateFilesFromFolders: " << sum / count << " micro seconds" << std::endl;

	delete manager;
	return 1;
}