#include "td_util.h"

td_util::Input::Input(char id) : _id(id) { }

td_util::Input::~Input() { }

char td_util::Input::getId() {
	return _id;
}

const std::string td_util::Input::getData() {
	return _data;
}
