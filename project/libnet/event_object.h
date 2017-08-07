#ifndef __event_object_h__
#define __event_object_h__

#include <string>

#include "base_type.h"
#include "object.h"

#include <event2/util.h>

struct event_base;

class EventObject : public Object
{
public:
	EventObject();
	virtual ~EventObject();

public:
	bool openEvent(bool fast = true);
	void closeEvent();

public:
	int dispatch();

public:
	int loopbreak();
	int loopexit(struct timeval delay = { 1, 0 });

public:
	virtual std::string debug();

protected:
	struct event_base* _base;
};


#endif // !__event_object_h__