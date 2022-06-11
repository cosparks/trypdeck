#include "MediaListener.h"

MediaListener::MediaListener() { }

MediaListener::~MediaListener() { }

void MediaListener::addFileIds(const std::vector<uint32_t>& ids) {
	for (uint32_t id : ids) {
		_addMedia(id);
	}
}

void MediaListener::update(const MediaChangedArgs& args) {
	switch (args.option) {
		case MediaChangedOptions::Added:
			_addMedia(args.fileId);
			break;
		case MediaChangedOptions::Modified:
			_updateMedia(args.fileId);
			break;
		case MediaChangedOptions::Removed:
			_removeMedia(args.fileId);
			break;
		default:
			// do nothing
			break;
	}
}

void MediaListener::addMediaFolder(const std::string folder) {
	_mediaFolders.push_back(folder);
}

const std::vector<std::string>& MediaListener::getMediaFolders() {
	return _mediaFolders;
}