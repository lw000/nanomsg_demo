#ifndef __event_object_h__
#define __event_object_h__

#include <string>

#include "base_type.h"
#include "object.h"

struct event_base;

class SocketProcessor : public Object
{
public:
	static void processorUseThreads();

public:
	SocketProcessor();
	virtual ~SocketProcessor();

public:
	bool create(bool enableServer);
	void destroy();

public:
	struct event_base* getBase();

public:
	int dispatch();

public:
	int loopbreak();
	int loopexit();

public:
	virtual std::string debug() override;

private:
	struct event_base* _base;
};


#endif // !__event_object_h__