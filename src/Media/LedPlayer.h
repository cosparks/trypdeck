#ifndef _LED_PLAYER_H_
#define _LED_PLAYER_H_

#include <string>
#include <vector>

#include "MediaListener.h"

class LedPlayer : public MediaListener {
	public:
		LedPlayer(const std::vector<std::string>& folders);
		~LedPlayer();

	private:
		void _addMedia(uint32_t fileId) override;
		void _removeMedia(uint32_t fileId) override;
		void _updateMedia(uint32_t fileId) override;
};

#endif