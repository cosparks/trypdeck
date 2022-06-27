#ifndef _TRIPDECK_BEHAVIOR_LEADER_H_
#define _TRIPDECK_BEHAVIOR_LEADER_H_

#include <string>
#include <vector>

#include "TripdeckBehavior.h"
#include "InputThreadedSerial.h"

class TripdeckBehaviorLeader : public TripdeckBehavior {
	public:
		TripdeckBehaviorLeader(InputManager* inputManager, Serial* serial);
		~TripdeckBehaviorLeader();
		void init() override;
		void run() override;
	private:
		std::vector<std::string> _nodeIds;
		InputThreadedSerial* _serialInput = NULL;
		SerialInputDelegate* _serialInputDelegate = NULL;

		void _onStateChanged(TripdeckStateChangedArgs& args) override;
		void _updateFollowers();
		void _handleSerialInput(InputArgs& args) override;
		void _handleUserInput(InputArgs* data);
		void _syncWithFollowers();
		void _checkInputs();
		void _notifyPulled();
		void _notifyReveal();
};

#endif