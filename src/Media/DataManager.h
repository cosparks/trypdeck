#ifndef _DATA_MANAGER_H_
#define _DATA_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sys/inotify.h>

#include "td_util.h"
#include "media.h"

#define EVENT_SIZE (sizeof(inotify_event))
#define BUFFER_LENGTH (64 * (EVENT_SIZE + 16))

/**
 * @brief Maintains representation of on-disk media files for project
 */
class DataManager {
	public:
		DataManager();
		DataManager(const std::vector<std::string>& paths);
		~DataManager();
		void init();
		void addFolderPath(const std::string path);

		// following methods must only be called after init
		void run();
		const std::vector<uint32_t>& getFileIdsFromFolder(const std::string& path);
		const std::vector<std::string> getSystemPathsFromFolder(const std::string& path);
		int32_t getNumFilesInFolder(const std::string& path);
		void addMediaListener(MediaListener* listener);
		void removeMediaListener(MediaListener* listener);
	private:
		int32_t _fd;
		std::unordered_map<int32_t, std::string> _watchDescriptorToFolder;
		std::unordered_map<int32_t, std::vector<MediaListener*>*> _watchDescriptorToListeners;
		std::unordered_map<std::string, std::vector<uint32_t>*> _folderToFileIds;
		uint8_t _notifyBuffer[BUFFER_LENGTH] = { };

		void _updateFilesFromFolders();
		void _readFileDescriptor();
		void _handleFolderChangedEvent(inotify_event* event);
		void _removeFileIdFromFolder(uint32_t id, const std::string folder);
		int32_t _getWatchDescriptorForFolder(const std::string& folder);
};

#endif