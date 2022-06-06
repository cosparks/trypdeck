#ifndef _VIDEO_MANAGER_H_
#define _VIDEO_MANAGER_H_

class VideoManager {
	public:
		VideoManager(DataManager& dataManager);
		~VideoManager();
		voit init();
		void run();
		void addFolder();
};

#endif