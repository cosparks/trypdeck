#ifndef _TD_UTIL_H_
#define _TD_UTIL_H_

#include <string>
#include <vector>

namespace td_util {
	typedef void* CommandArgs;
	class Command {
		public:
			virtual ~Command() { }
			virtual void execute(CommandArgs args) = 0;
	};

	class Input {
		public:
			Input(char id);
			virtual ~Input();
			virtual bool read() = 0;
			char getId();
			const std::string getData();
		protected:
			char _id;
			std::string _data;
	};

	struct InputArgs {
		char id;
		std::string buffer;
	};
}

#endif