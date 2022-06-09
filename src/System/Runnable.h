#ifndef _RUNNABLE_H_
#define _RUNNABLE_H_

class Runnable {
	public:
		virtual ~Runnable() { }
		virtual void init() = 0;
		virtual void run() = 0;
};

#endif