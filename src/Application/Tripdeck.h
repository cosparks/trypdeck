#ifndef _TRIPDECK_H_
#define _TRIPDECK_H_

#include "InputManager.h"

class Tripdeck {
	public:
		Tripdeck();
		~Tripdeck();
		void init();
		void run();
	private:
		enum TripdeckState { Startup, Wait, Pulled, Reveal };
		bool _run;
};

#endif