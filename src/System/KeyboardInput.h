#ifndef _KEYBOARD_INPUT_H_
#define _KEYBOARD_INPUT_H_

#include <ncurses.h>

#include "Input.h"

template <class T>
class KeyboardInput : public Input<char> {
	public:
		KeyboardInput() { }
		~KeyboardInput() {
			endwin();
		}

		void init() override {
			initscr();
		}

		void run() override {
			nodelay(stdscr, TRUE);
			noecho();

			char input = getch();
			if (input != 255) {
				(_object->*_callback)(input);
				ungetch(input);
			}

			echo();
			nodelay(stdscr, FALSE);
		}

		void setCallback(T* object, void (T::*callback)(char)) {
			_object = object;
			_callback = callback;
		}

	private:
		T* _object;
		void (T::*_callback)(char);
		uint32_t (Apa102::*_getIndexFromPoint)(const Point&);
};

#endif