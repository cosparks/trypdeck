#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include <stdint.h>

typedef void* NotifyData;
struct NotifyArgs {
	int32_t id;
	NotifyData data;
};

class Observer {
	public:
		Observer();
		~Observer();
		virtual void notify(NotifyArgs args);
};

#endif
