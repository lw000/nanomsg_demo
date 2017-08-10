#include "socket_processor.h"

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/util.h>
#include <event2/thread.h>

SocketProcessor::SocketProcessor()
{

}

SocketProcessor::~SocketProcessor()
{
}

bool SocketProcessor::open(bool enableServer)
{
	if (this->_base == nullptr)
	{
		if (enableServer)
		{
#ifdef WIN32
			//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
			evthread_use_windows_threads();

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
		else
		{
			this->_base = event_base_new();
		}
	}

	return true;
}

void SocketProcessor::close()
{
	if (this->_base != nullptr)
	{
		event_base_free(this->_base);
		this->_base = nullptr;
	}
}

struct event_base* SocketProcessor::getBase()
{ 
	return this->_base;
}

int SocketProcessor::dispatch()
{
	int r = event_base_dispatch(this->_base);
	return r;
}

int SocketProcessor::loopbreak()
{
	int r = event_base_loopbreak(this->_base);
	return r;
}

int SocketProcessor::loopexit()
{
	struct timeval delay = { 1, 0 };
	int r = event_base_loopexit(this->_base, &delay);
	return r;
}

std::string SocketProcessor::debug()
{
	return std::string("EventObject");
}
