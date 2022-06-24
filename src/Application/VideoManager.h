#ifndef _VIDEO_MANAGER_H_
#define _VIDEO_MANAGER_H_

#include "DataManager.h"

class VideoManager {
	public:
		VideoManager(DataManager& dataManager);
		~VideoManager();
		void init();
		void run();
		void addFolder();
};

#endif