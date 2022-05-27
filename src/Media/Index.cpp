#include "Index.h"

Index* Index::_instance;

Index& Index::instance() {
	if (_instance == nullptr) {
		_instance = new Index();
	}
	return *_instance;
}

Index::Index() { }

Index::~Index() {
	delete _instance;
}

const std::string& Index::getSystemPath(uint32_t id) {
	return _idToSystemPath[id];
}

const std::string& Index::getSystemPath(const std::string& fileName) {
	return _idToSystemPath[std::hash<std::string>{}(fileName)];
}

uint32_t Index::addFile(std::string folder, const std::string& fileName) {
	folder.append(fileName);
	uint32_t id = std::hash<std::string>{}(fileName);
	_idToSystemPath[id] = folder;
	return id;
}

void Index::removeFile(uint32_t id) {
	_idToSystemPath.erase(id);
}

uint32_t Index::removeFile(const std::string& fileName) {
	uint32_t id = std::hash<std::string>{}(fileName);
	_idToSystemPath.erase(id);
	return id;
}