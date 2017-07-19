#include "object.h"

Object::Object()
{
	_data = NULL;
	_tag = -1;
}

Object::~Object()
{
}

void Object::setUserData(void* data)
{
	if (this->_data != data)
	{
		this->_data = data;
	}
}

void* Object::getUserData()
{
	return _data;
}

void Object::setTag(int tag)
{
	this->_tag = tag;
}

int Object::getTag()
{
	return this->_tag;
}

std::string Object::debug()
{
	return std::string("Object");
}
