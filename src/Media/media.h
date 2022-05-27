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
		virtual void updateMedia(MediaChangedArgs args) = 0;
		virtual const std::vector<std::string>& getMediaFolders() = 0;
};

#endif