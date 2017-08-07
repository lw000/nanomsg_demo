#include "event_object.h"

#include "event2/event.h"

EventObject::EventObject()
{
}

EventObject::~EventObject()
{
}

bool EventObject::openEvent(bool fast)
{
	if (this->_base == nullptr)
	{
#ifdef WIN32
		if (fast)
		{
			struct event_config *cfg = event_config_new();
			event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
			if (cfg)
			{
				this->_base = event_base_new_with_config(cfg);
				event_config_free(cfg);
			}
		}
		else
		{
			this->_base = event_base_new();
		}
#else
		this->_base = event_base_new();
#endif	
	}

	return true;
}

void EventObject::closeEvent()
{
	if (this->_base != nullptr)
	{
		event_base_free(this->_base);
		this->_base = nullptr;
	}
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

int EventObject::loopexit(struct timeval delay)
{
	int r = event_base_loopexit(this->_base, &delay);
	return r;
}

std::string EventObject::debug()
{
	return std::string("EventObject");
}
