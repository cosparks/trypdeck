#ifndef _INDEX_H_
#define _INDEX_H_

#include <functional>
#include <unordered_map>

/**
 * @brief singleton class which maintains id -> system path mappings
 */
class Index {
	public:
		static Index& instance();
		~Index();
		const std::string& getSystemPath(uint32_t id);
		const std::string& getSystemPath(const std::string& fileName);
		uint32_t getFileId(const std::string& fileName);

		// danger zone -- only call this method from DataManager!
		uint32_t addFile(std::string folder, const std::string& fileName);
		// danger zone -- only call this method from DataManager!
		void removeFile(uint32_t id);
		// danger zone -- only call this method from DataManager!
		uint32_t removeFile(const std::string& fileName);
	private:
		static Index* _instance;
		std::unordered_map<uint32_t, std::string> _idToSystemPath;

		Index();
};

#endif