#include <string.h>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "Index.h"
#include "DataManager.h"

#define SELECT_TIMEOUT_MICROS 250

DataManager::DataManager() { }

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
		throw std::runtime_error("Error: unable to initialize inotify");
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
	for (const std::string folderPath : listener->getMediaFolders()) {
		// add media folder and its files and provide file ids to listener
		int32_t wd = _addFolder(folderPath);
		listener->addFileIds(*_folderToFileIds[folderPath]);
		
		// add listener to watchDescriptor-to-listener cache
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

		if (vec->size() == 0) {
			// remove folder and watch descriptor
			_removeFolder(folderPath);
			inotify_rm_watch(_fd, wd);
			_watchDescriptorToListeners.erase(wd);
			_watchDescriptorToFolder.erase(wd);
		}
	}
}

const std::vector<uint32_t>& DataManager::getFileIdsFromFolder(const std::string& path) {
	return *_folderToFileIds[path];
}

int32_t DataManager::getNumFilesInFolder(const std::string& path) {
	return _folderToFileIds[path]->size();
}

int32_t DataManager::_addFolder(const std::string path) {
	if (_folderToFileIds.find(path) == _folderToFileIds.end()) {
		// create new entry in folder-to-fileId map
		_folderToFileIds[path] = new std::vector<uint32_t>();

		#if ENABLE_FILE_SYSTEM_DEBUG
		// TODO: remove debug code
		std::cout << "Adding folder to watch " << path << std::endl;
		#endif

		// add inotify watch for folder
		int32_t wd = inotify_add_watch(_fd, path.c_str(), IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_MODIFY);
		if (wd < 0) {
			throw std::runtime_error("inotify error! unable to add watch: " + std::string(strerror(errno)) +  "\n\tdir: " + path);
		}

		_watchDescriptorToFolder[wd] = path;

		// add files from folder to Index and member file
		_addFilesFromFolder(path);
		return wd;
	} else {
		return _getWatchDescriptorForFolder(path);
	}
}

void DataManager::_removeFolder(const std::string& path) {
	if (_folderToFileIds.find(path) != _folderToFileIds.end()) {
		delete _folderToFileIds[path];
		_folderToFileIds.erase(path);
	}
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
		// notify file removed and remove file from folder
		_updateListeners(event, MediaChangedArgs { Index::instance().getFileId(event->name), MediaChangedOptions::Removed });
		_removeFileIdFromFolder(Index::instance().removeFile(event->name), _watchDescriptorToFolder[event->wd]);
	}

	if (event->mask & IN_MOVED_TO || event->mask & IN_CREATE) {
		// add file to folder and notify file added
		std::string folder = _watchDescriptorToFolder[event->wd];
		uint32_t fileId = Index::instance().addFile(folder, event->name);
		_folderToFileIds[folder]->push_back(fileId);
		_updateListeners(event, MediaChangedArgs { fileId, MediaChangedOptions::Added });
	}

	if (event->mask & IN_MODIFY) {
		// notify file modified
		_updateListeners(event, MediaChangedArgs { Index::instance().getFileId(event->name), MediaChangedOptions::Modified });
	}
}

void DataManager::_updateListeners(inotify_event* event, const MediaChangedArgs& args) {
	for (MediaListener* listener : *_watchDescriptorToListeners[event->wd]) {
		listener->update(args);
	}
}

void DataManager::_addFilesFromFolder(const std::string& path) {
	DIR *dir;
	dirent *ent;
	class stat st;

	dir = opendir(path.c_str()); 

	if (dir == nullptr)
		throw std::runtime_error("Error: folder does not exist: " + path);

	// get all files from folder
	while ((ent = readdir(dir)) != NULL) {
		const std::string fileName = ent->d_name;
		const std::string systemPath = path + fileName;

		if (fileName[0] == '.')
			continue;

		if (stat(systemPath.c_str(), &st) == -1)
			continue;

		const bool is_directory = (st.st_mode & S_IFDIR) != 0;

		if (is_directory)
			continue;

		// add file to Index and file ID to internal folder map
		_folderToFileIds[path]->push_back(Index::instance().addFile(path, fileName));
	}

	closedir(dir);
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