#include "MediaManager.h"

#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

enum FileValidationOption { Move, Remove, DoNothing };
struct FileValidationData {
	FileValidationOption option;
	std::string folderPath;
};

MediaManager::MediaManager() { }

MediaManager::MediaManager(const std::vector<std::string>& paths) {
	for (const std::string path : paths) {
		addFolderPath(path);
	}
}

MediaManager::~MediaManager() {
	for (const auto& pair : _folderToFileNames) {
		delete pair.second;
	}
}

void MediaManager::addFolderPath(const std::string path) {
	if (_folderToFileNames.find(path) == _folderToFileNames.end())
		_folderToFileNames[path] = new std::vector<std::string>();
}

const std::vector<std::string>& MediaManager::getFileUrlsFromFolder(const std::string& folderPath) {
	return *_folderToFileNames[folderPath];
}

void MediaManager::init() {
	_updateFilesFromFolders(true);
}

void MediaManager::updateFilesFromFolders() {
	_updateFilesFromFolders(false);
}

void MediaManager::_updateFilesFromFolders(bool init) {
	std::unordered_map<std::string, FileValidationData> fileValidationMap;

	// don't need to update application state at initialization
	if (!init) {
		// set up file validation map, assume that we will remove the file
		for (const auto& pair : _folderToFileNames) {
			for (const std::string& name : *pair.second) {
				fileValidationMap[name] = FileValidationData { Remove, pair.first };
			}
		}
	}

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
				fileValidationMap[fileName] = FileValidationData { DoNothing, pair.first };
			} else {
				if (fileValidationMap[fileName].folderPath.compare(pair.first) != 0) {
					fileValidationMap[fileName] = FileValidationData { Move, pair.first };
				} else {
					fileValidationMap[fileName] = FileValidationData { DoNothing, pair.first };
				}
			}

			// set FileValidationData.keep to true and update file folder (in case file was moved from one folder to another)
		}
		closedir(dir);
	}


	// don't need to update application state at initialization
	if (!init) {
		// check to see if we need to remove any files from cache
		for (const auto& pair : fileValidationMap) {
			switch (pair.second.option) {
				case Move:
					_removeFileFromFolder(pair.first, _getFolderFromFileName(pair.first));					
					_folderToFileNames[pair.second.folderPath]->push_back(pair.first);
					_fileNameToSystemPath[pair.first] = pair.second.folderPath + pair.first;
					break;
				case Remove:
					// remove entry from _fileToSystemPath and _folderToFileNames cache
					_fileNameToSystemPath.erase(pair.first);
					_removeFileFromFolder(pair.first, pair.second.folderPath);
					break;
				default:
					// do nothing
					break;
			}
		}
	}
}

const std::string MediaManager::_getFolderFromFileName(const std::string& file) {
	return _fileNameToSystemPath[file].substr(0, _fileNameToSystemPath[file].find(file));
}

void MediaManager::_removeFileFromFolder(const std::string& file, const std::string folder) {
	auto vec = _folderToFileNames[folder];
	vec->erase(std::remove(vec->begin(), vec->end(), file), vec->end());
}