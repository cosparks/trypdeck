#ifndef _MEDIA_LISTENER_H_
#define _MEDIA_LISTENER_H_

#include <string>
#include <vector>
#include <stdint.h>

#include "Runnable.h"

enum MediaChangedOptions { Added, Removed, Modified };
struct MediaChangedArgs {
	uint32_t fileId;
	MediaChangedOptions option;
};

class MediaListener : public Runnable {
	public:
		MediaListener();
		virtual ~MediaListener();
		void addFileIds(const std::vector<uint32_t>& ids);
		void update(const MediaChangedArgs& args);
		void addMediaFolder(const std::string folder);
		const std::vector<std::string>& getMediaFolders();

	protected:
		std::vector<std::string> _mediaFolders;

		virtual void _addMedia(uint32_t fileId) = 0;
		virtual void _removeMedia(uint32_t fileId) = 0;
		virtual void _updateMedia(uint32_t fileId) = 0;
};

#endif