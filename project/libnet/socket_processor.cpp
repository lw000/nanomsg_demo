#include "socket_processor.h"

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/util.h>
#include <event2/thread.h>

void SocketProcessor::processorUseThreads()
{
#ifdef WIN32
	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
	evthread_use_windows_threads();
#else
	evthread_use_pthreads();
#endif	
}

SocketProcessor::SocketProcessor()
{

}

SocketProcessor::~SocketProcessor()
{
	
}

bool SocketProcessor::create(bool enableServer, SocketCore* core)
{
	this->_core = core;

	if (this->_base == nullptr)
	{
		if (enableServer)
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
		else
		{
			this->_base = event_base_new();
		}
	}

	return true;
}

void SocketProcessor::destroy()
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

SocketCore* SocketProcessor::getSocketCore()
{
	return this->_core;
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
	struct timeval delay = {0, 5000000};
	int r = event_base_loopexit(this->_base, &delay);
	return r;
}

std::string SocketProcessor::debug()
{
	char buf[256];
	if (this->_base == nullptr)
	{
		sprintf(buf, "[SocketProcessor: %x]", this);
	}
	else
	{
		sprintf(buf, "[SocketProcessor: %x, %x]", this, this->_base);
	}
	return std::string(buf);
}
