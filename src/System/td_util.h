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
			Input(uint32_t id);
			virtual ~Input();
			virtual bool read() = 0;
			uint32_t getId();
			const std::string& getData();
		protected:
			uint32_t _id;
			std::string _data;
	};

	struct InputArgs {
		uint32_t id;
		const std::string& buffer;
	};
}

#endif