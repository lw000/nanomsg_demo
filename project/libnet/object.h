#ifndef __object_h__
#define __object_h__

#include "base_type.h"
#include <string>

class Object
{
public:
	Object();
	virtual ~Object();

public:
	void setUserData(void* data);
	void* getUserData();

	void setTag(int tag);
	int getTag();

public:
	virtual std::string debug();

private:
	void *_data;
	int _tag;
};


#endif // !__object_h__