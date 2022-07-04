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

	template <class T, class R>
	class Delegate : public Command {
		public:
			Delegate(T* owner, void (T::*callback)(const R&)) : _owner(owner), _callback(callback) { }
			~Delegate() { }
			void execute(CommandArgs args) override {
				(_owner->*_callback)(*((R*)args));
			}
		private:
			T* _owner;
			void (T::*_callback)(const R&);
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

	template <class T>
	void onThreadExit(T* obj, void (T::*callback)(void)) {
		class ThreadExiter {
			public:
				ThreadExiter() = default;
				ThreadExiter(ThreadExiter const&) = delete;
				void operator=(ThreadExiter const&) = delete;
				~ThreadExiter() {
					(_object->*_exitCallback)();
				}
				void setCallback(T* object, void (T::*exitCallback)(void)) {
					_object = object;
					_exitCallback = exitCallback;
				}
			private:
				T* _object;
				void (T::*_exitCallback)(void);
		};

		thread_local ThreadExiter exiter;
		exiter.setCallback(obj, callback);
	}
}

#endif