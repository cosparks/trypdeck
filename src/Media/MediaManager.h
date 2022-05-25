#ifndef _MEDIA_MANAGER_H_
#define _MEDIA_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief Manages media files from a particular folder/set of folders
 */
class MediaManager {
	public:
		MediaManager();
		MediaManager(const std::vector<std::string>& paths);
		~MediaManager();
		void init();
		void addFolderPath(const std::string path);
		const std::vector<std::string>& getFileUrlsFromFolder(const std::string& folderPath);
		void updateFilesFromFolders();
	private:
		std::unordered_map<std::string, std::vector<std::string>*> _folderToFileNames;
		std::unordered_map<std::string, std::string> _fileNameToSystemPath;

		void _updateFilesFromFolders(bool init);
		const std::string _getFolderFromFileName(const std::string& file);
		void _removeFileFromFolder(const std::string& file, const std::string folder);
};

#endif