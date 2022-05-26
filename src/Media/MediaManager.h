#ifndef _MEDIA_MANAGER_H_
#define _MEDIA_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(inotify_event))
#define BUFFER_LENGTH (64 * (EVENT_SIZE + 16))

/**
 * @brief Maintains representation of on-disk media files for project
 */
class MediaManager {
	public:
		MediaManager();
		MediaManager(const std::vector<std::string>& paths);
		~MediaManager();
		void init();
		void run();
		void addFolderPath(const std::string path);
		const std::vector<std::string>& getFileUrlsFromFolder(const std::string& folderPath);
		int32_t getNumFilesInFolder(const std::string& folderPath);
		const std::string& getSystemPathFromFileName(const std::string& fileName);
	private:
		int32_t _fd;
		std::unordered_map<int32_t, std::string> _watchDescriptorToFolder;
		std::unordered_map<std::string, std::vector<std::string>*> _folderToFileNames;
		std::unordered_map<std::string, std::string> _fileNameToSystemPath;
		uint8_t _notifyBuffer[BUFFER_LENGTH] = { };

		void _updateFilesFromFolders();
		void _readFileDescriptor();
		void _handleFolderChangedEvent(inotify_event* event);
		const std::string _getFolderFromFileName(const std::string& file);
		void _removeFileFromFolder(const std::string& file, const std::string folder);
};

#endif