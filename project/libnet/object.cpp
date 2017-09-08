#include "object.h"

std::ostream& operator<<(std::ostream & os, Object & o) {
	os << o.debug();
	return os;
}

Object::Object() {
	_data = nullptr;
	_tag = -1;
}

Object::~Object() {
}

void Object::setUserData(void* data) {
	if (this->_data != data) {
		this->_data = data;
	}
}

void* Object::getUserData() const {
	return _data;
}

void Object::setTag(int tag) {
	this->_tag = tag;
}

int Object::getTag() const {
	return this->_tag;
}

void Object::setName(const std::string& name) {
	if (this->_name != name) {
		this->_name = name;
	}
}

std::string Object::getName() const {
	return this->_name;
}

std::string Object::debug()
{
	return std::string("Object");
}
