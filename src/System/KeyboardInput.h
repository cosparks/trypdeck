// #ifndef _KEYBOARD_INPUT_H_
// #define _KEYBOARD_INPUT_H_

// #include <ncurses.h>

// #include "Runnable.h"

// template <class T>
// class KeyboardInput : public Runnable {
// 	public:
// 		KeyboardInput(T* object, void (T::*callback)(int32_t)) {
// 			_object = object;
// 			_callback = callback;
// 		}

// 		~KeyboardInput() {
// 			endwin();
// 		}

// 		void init() override {
// 			initscr();
// 		}

// 		void run() override {
// 			nodelay(stdscr, TRUE);
// 			noecho();
// 			bool keyDown = false;
// 			int32_t input = getch();

// 			if (input == ERR) {
// 				// do nothing
// 			} else {
// 				keyDown = true;
// 				ungetch(input);
// 			}

// 			echo();
// 			nodelay(stdscr, FALSE);

// 			if (keyDown) {
// 				(_object->*_callback)(input);
// 			}
// 		}

// 	private:
// 		T* _object;
// 		void (T::*_callback)(int32_t);
// };

// #endif