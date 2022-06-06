#ifndef _TD_UTIL_H_
#define _TD_UTIL_H_

#include <string>
#include <vector>

namespace td_util {
	const std::string getVideoPath(const std::string& path);
	const std::string getAnimPath(const std::string& path);

	typedef void* CommandArgs;
	class Command {
		public:
			virtual ~Command() { }
			virtual void execute(CommandArgs args) = 0;
	};

	typedef void* UpdateArgs;
	class Observer {
		public:
			virtual ~Observer() { };
			virtual void notify(UpdateArgs args) = 0;
	};

	class ObservableObject {
		public:
			virtual ~ObservableObject() { }
		protected:
			std::vector<Observer*> _observers;
			virtual void notify() = 0;
	};
}

#endif