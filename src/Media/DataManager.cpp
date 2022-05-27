#include <string.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "DataManager.h"
#include "Index.h"

#define SELECT_TIMEOUT_MICROS 250

DataManager::DataManager() { }

DataManager::DataManager(const std::vector<std::string>& folders) {
	for (const std::string folderPath : folders) {
		addFolderPath(folderPath);
	}
}

DataManager::~DataManager() {
	for (const auto& pair : _folderToFileIds)
		delete pair.second;

	for (const auto& pair : _watchDescriptorToListeners)
		delete pair.second;

	for (const auto& pair : _watchDescriptorToFolder)
		inotify_rm_watch(_fd, pair.first);

	if (close(_fd) < 0)
		perror("Failed to properly close file descriptor in DataManager");
}

void DataManager::init() {
	// set up inotify
	_fd = inotify_init();
	if (_fd < 0)
		throw std::runtime_error("Error: unable to initialize inotify_init");

	for (const auto& pair : _folderToFileIds) {
		std::cout << "Adding folder to watch: " << pair.first << std::endl;
		int32_t wd = inotify_add_watch(_fd, pair.first.c_str(), IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_MODIFY);

		if (wd < 0)
			throw std::runtime_error("Error: unable to add watch to dir " + pair.first);

		_watchDescriptorToFolder[wd] = pair.first;
	}
	_updateFilesFromFolders();
}

void DataManager::addFolderPath(const std::string path) {
	if (_folderToFileIds.find(path) == _folderToFileIds.end())
		_folderToFileIds[path] = new std::vector<uint32_t>();
}

void DataManager::run() {
	timeval time = { 0, SELECT_TIMEOUT_MICROS };
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_fd, &rfds);

	// check if inotify file descriptor is ready to be read
	int32_t ret = select(_fd + 1, &rfds, NULL, NULL, &time);
	if (ret < 0) {
		throw std::runtime_error("Error: select returned error code " + ret);
	} else if (ret != 0 && FD_ISSET(_fd, &rfds)) {
		_readFileDescriptor();
	}
}

void DataManager::addMediaListener(MediaListener* listener) {
	for (const std::string& folderPath : listener->getMediaFolders()) {
		int32_t wd = _getWatchDescriptorForFolder(folderPath);

		if (wd < 0)
			continue;
		
		if (_watchDescriptorToListeners.find(wd) == _watchDescriptorToListeners.end())
			_watchDescriptorToListeners[wd] = new std::vector<MediaListener*>();
		
		_watchDescriptorToListeners[wd]->push_back(listener);
	}
}

void DataManager::removeMediaListener(MediaListener* listener) {
	for (const std::string& folderPath : listener->getMediaFolders()) {
		int32_t wd = _getWatchDescriptorForFolder(folderPath);

		if (wd < 0)
			continue;

		if (_watchDescriptorToListeners.find(wd) == _watchDescriptorToListeners.end())
			continue;

		auto vec = _watchDescriptorToListeners[wd];
		vec->erase(std::remove(vec->begin(), vec->end(), listener), vec->end());
	}
}

const std::vector<uint32_t>& DataManager::getFileIdsFromFolder(const std::string& path) {
	return *_folderToFileIds[path];
}

int32_t DataManager::getNumFilesInFolder(const std::string& path) {
	return _folderToFileIds[path]->size();
}

void DataManager::_readFileDescriptor() {
	int32_t len = read(_fd, _notifyBuffer, BUFFER_LENGTH);

	if (len < 0) {
		perror("DataManager read error");
	} else if (!len) {
		perror("DataManager notify buffer too small");
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

void DataManager::_handleFolderChangedEvent(inotify_event* event) {
	if (event->mask & IN_MOVED_FROM || event->mask & IN_DELETE) {
		// remove file from folder
		_removeFileIdFromFolder(Index::instance().removeFile(event->name), _watchDescriptorToFolder[event->wd]);
	}

	if (event->mask & IN_MOVED_TO || event->mask & IN_CREATE) {
		// add file to folder
		std::string folder = _watchDescriptorToFolder[event->wd];
		_folderToFileIds[folder]->push_back(Index::instance().addFile(folder, event->name));
	}

	if (event->mask & IN_MODIFY) {
		// send out file modified notification

	}
}

void DataManager::_updateFilesFromFolders() {
	// get all files from folders
	for (const auto& pair : _folderToFileIds) {
		DIR *dir;
		dirent *ent;
		class stat st;

		dir = opendir(pair.first.c_str());

		if (dir == nullptr)
			continue;

		while ((ent = readdir(dir)) != NULL) {
			const std::string fileName = ent->d_name;
			const std::string systemPath = pair.first + fileName;

			if (fileName[0] == '.')
				continue;

			if (stat(systemPath.c_str(), &st) == -1)
				continue;

			const bool is_directory = (st.st_mode & S_IFDIR) != 0;

			if (is_directory)
				continue;

			// add file to Index and file ID to internal folder map
			pair.second->push_back(Index::instance().addFile(pair.first, fileName));
		}
		closedir(dir);
	}
}

void DataManager::_removeFileIdFromFolder(uint32_t id, const std::string folder) {
	auto vec = _folderToFileIds[folder];
	vec->erase(std::remove(vec->begin(), vec->end(), id), vec->end());
}

int32_t DataManager::_getWatchDescriptorForFolder(const std::string& folder) {
	int32_t wd = -1;
	for (const auto& pair : _watchDescriptorToFolder) {
		if (folder.compare(pair.second) == 0)
			return pair.first;
	}
	return wd;
}