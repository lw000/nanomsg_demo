#include "event_object.h"

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/util.h>

EventObject::EventObject()
{
}

EventObject::~EventObject()
{
}

bool EventObject::openClient()
{
	if (this->_base == nullptr)
	{
		this->_base = event_base_new();
	}

	return true;
}

bool EventObject::openServer()
{
	if (this->_base == nullptr)
	{
#ifdef WIN32
		struct event_config *cfg = event_config_new();
		event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
		if (cfg)
		{
			this->_base = event_base_new_with_config(cfg);
			event_config_free(cfg);
		}
#else
		this->_base = event_base_new();
#endif	
	}

	return true;
}

void EventObject::close()
{
	if (this->_base != nullptr)
	{
		event_base_free(this->_base);
		this->_base = nullptr;
	}
}

struct event_base* EventObject::getBase()
{ 
	return this->_base;
}

int EventObject::dispatch()
{
	int r = event_base_dispatch(this->_base);
	return r;
}

int EventObject::loopbreak()
{
	int r = event_base_loopbreak(this->_base);
	return r;
}

int EventObject::loopexit()
{
	struct timeval delay = { 1, 0 };
	int r = event_base_loopexit(this->_base, &delay);
	return r;
}

std::string EventObject::debug()
{
	return std::string("EventObject");
}
