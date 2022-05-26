#include "MediaManager.h"

#include <string.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#define SELECT_TIMEOUT_MICROS 500

enum FileChangedOption { MoveToFolder, RemoveFromFolder, DoNothing };
struct FileChangedData {
	FileChangedOption option;
	std::string folderPath;
};

MediaManager::MediaManager() { }

MediaManager::MediaManager(const std::vector<std::string>& paths) {
	for (const std::string path : paths) {
		addFolderPath(path);
	}
}

MediaManager::~MediaManager() {
	for (const auto& pair : _folderToFileNames)
		delete pair.second;

	for (const auto& pair : _watchDescriptorToFolder)
		inotify_rm_watch(_fd, pair.first);

	if (close(_fd) < 0)
		perror("Failed to properly close file descriptor in MediaManager");
}

void MediaManager::init() {
	// set up inotify
	_fd = inotify_init();
	if (_fd < 0)
		throw std::runtime_error("Error: unable to initialize inotify_init");

	for (const auto& pair : _folderToFileNames) {
		std::cout << "Adding folder to watch: " << pair.first << std::endl;
		int32_t wd = inotify_add_watch(_fd, pair.first.c_str(), IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE);

		if (wd < 0)
			throw std::runtime_error("Error: unable to add watch to dir " + pair.first);

		_watchDescriptorToFolder[wd] = pair.first;
	}
	_updateFilesFromFolders();
}

void MediaManager::run() {
	timeval time = { 0, SELECT_TIMEOUT_MICROS };
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_fd, &rfds);

	int32_t ret = select(_fd + 1, &rfds, NULL, NULL, &time);
	if (ret < 0) {
		throw std::runtime_error("Error: select returned error code " + ret);
	} else if (ret != 0 && FD_ISSET(_fd, &rfds)) {
		_readFileDescriptor();
	}
}

void MediaManager::addFolderPath(const std::string path) {
	if (_folderToFileNames.find(path) == _folderToFileNames.end())
		_folderToFileNames[path] = new std::vector<std::string>();
}

const std::vector<std::string>& MediaManager::getFileUrlsFromFolder(const std::string& folderPath) {
	return *_folderToFileNames[folderPath];
}

int32_t MediaManager::getNumFilesInFolder(const std::string& folderPath) {
	return _folderToFileNames[folderPath]->size();
}

const std::string& MediaManager::getSystemPathFromFileName(const std::string& fileName) {
	return _fileNameToSystemPath[fileName];
}

void MediaManager::_readFileDescriptor() {
	int32_t len = read(_fd, _notifyBuffer, BUFFER_LENGTH);

	if (len < 0) {
		perror("MediaManager read error");
	} else if (!len) {
		perror("MediaManager notify buffer too small");
	}
	int32_t i = 0;
	while (i < len) {
		inotify_event *event = (inotify_event*)&_notifyBuffer[i];

		if (event->len > 0) {
			_handleFolderChangedEvent(event);
		}
		i += EVENT_SIZE + event->len;
	}
	memset(_notifyBuffer, 0, BUFFER_LENGTH);
}

void MediaManager::_handleFolderChangedEvent(inotify_event* event) {
	bool added = false;

	if (event->mask & IN_MOVED_TO || event->mask & IN_CREATE) {
		std::string folder = _watchDescriptorToFolder[event->wd];

		// add file to folder
		_folderToFileNames[folder]->push_back(event->name);
		_fileNameToSystemPath[event->name] = folder + event->name;
		added = true;
	}

	if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
		// remove file from folder
		_removeFileFromFolder(event->name, _watchDescriptorToFolder[event->wd]);

		// only need to remove from name to full path map if file has been removed from all project folders
		if (!added) {
			_fileNameToSystemPath.erase(event->name);
		}
	}
}

void MediaManager::_updateFilesFromFolders() {
	// get all files from folders
	for (const auto& pair : _folderToFileNames) {
		DIR *dir;
		dirent *ent;
		class stat st;

		dir = opendir(pair.first.c_str());

		if (dir == nullptr)
			continue;

		while ((ent = readdir(dir)) != NULL) {
			const std::string fileName = ent->d_name;
			const std::string fullFileName = pair.first + fileName;

			if (fileName[0] == '.')
				continue;

			if (stat(fullFileName.c_str(), &st) == -1)
				continue;

			const bool is_directory = (st.st_mode & S_IFDIR) != 0;

			if (is_directory)
				continue;

			if (_fileNameToSystemPath.find(fileName) == _fileNameToSystemPath.end()) {
				pair.second->push_back(fileName);
				_fileNameToSystemPath[fileName] = fullFileName;
			}
			// set FileChangedData.keep to true and update file folder (in case file was moved from one folder to another)
		}
		closedir(dir);
	}
}

const std::string MediaManager::_getFolderFromFileName(const std::string& file) {
	return _fileNameToSystemPath[file].substr(0, _fileNameToSystemPath[file].find(file));
}

void MediaManager::_removeFileFromFolder(const std::string& file, const std::string folder) {
	auto vec = _folderToFileNames[folder];
	vec->erase(std::remove(vec->begin(), vec->end(), file), vec->end());
}