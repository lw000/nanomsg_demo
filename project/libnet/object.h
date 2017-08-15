#ifndef __object_h__
#define __object_h__

#include "base_type.h"
#include <string>
#include <iosfwd>

class Object
{
	friend std::ostream& operator<<(std::ostream & os, Object & o);

public:
	Object();
	virtual ~Object();

public:
	void setUserData(void* data);
	void* getUserData();

	void setTag(int tag);
	int getTag();

public:
	virtual std::string debug() = 0;

private:
	void *_data;
	int _tag;
};


#endif // !__object_h__