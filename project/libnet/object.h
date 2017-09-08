#ifndef __object_h__
#define __object_h__

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
	void* getUserData() const;

	void setTag(int tag);
	int getTag() const;

	void setName(const std::string& name);
	std::string getName() const;

public:
	virtual std::string debug() = 0;

private:
	void *_data;
	int _tag;
	std::string _name;
};


#endif // !__object_h__