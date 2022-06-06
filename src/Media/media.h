#ifndef _MEDIA_H_
#define _MEDIA_H_

#include <vector>
#include <stdint.h>

enum MediaChangedOptions { Added, Removed, Modified };
struct MediaChangedArgs {
	uint32_t fileId;
	MediaChangedOptions option;
};

class MediaListener {
	public:
		virtual ~MediaListener() { }
		virtual void addFileIds(const std::vector<uint32_t>& ids) = 0;
		virtual void updateMedia(const MediaChangedArgs& args) = 0;
		virtual const std::vector<std::string>& getMediaFolders() = 0;
};

#endif