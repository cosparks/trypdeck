#ifndef _MEDIA_LISTENER_DUMMY_H_
#define _MEDIA_LISTENER_DUMMY_H_

#include "MediaListener.h"

// Temporary dummy class which is currently necessary given application data-management system where
// every node needs to have the exact same representation of the current file state of the other nodes
class MediaListenerDummy : public MediaListener {
	public:
		MediaListenerDummy() { }
		~MediaListenerDummy() { }
		void init() override {
			// do nothing
		}

		void run() override {
			// do nothing
		}

	private:
		void _addMedia(uint32_t fileId) override {
			// do nothing
		}

		void _removeMedia(uint32_t fileId) override {
			// do nothing
		}

		void _updateMedia(uint32_t fileId) override {
			// do nothing
		}
};

#endif