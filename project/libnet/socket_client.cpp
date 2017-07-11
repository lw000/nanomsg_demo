#include "socket_client.h"

// #if defined(WIN32) || defined(_WIN32)
// #include <winsock2.h>
// #endif // WIN32

#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "business.h"
#include "session.h"

using namespace LW;

#define BUF_SIZE	1024

static void time_cb(evutil_socket_t fd, short event, void *arg);

static void time_cb(evutil_socket_t fd, short event, void *arg)
{
	SocketClient *client = (SocketClient*)arg;
	client->time_cb(fd, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient()
{
	_session = new SocketSession(SocketSession::TYPE::Client);
}

SocketClient::~SocketClient()
{
	delete _session;
}

bool SocketClient::init()
{
	return true;
}

int SocketClient::run(const char* addr, int port)
{
	if (addr == NULL) return -1;

	this->_session->host = addr;
	this->_session->port = port;

	std::thread t(std::bind(&SocketClient::__run, this));
	t.detach();

	return 0;
}

bool SocketClient::isConnected()
{ 
	return this->_session->connected;
}

int SocketClient::setRecvHook(LW_PARSE_DATA_CALLFUNC func)
{
	this->_session->setRecvCall(func);

	return 0;
}

int SocketClient::setTimerHook(std::function<bool(SocketSession* session)> func)
{
 	if (func == NULL) return -1;

	this->_on_timer_func = func;

	return 0;
}

void SocketClient::time_cb(evutil_socket_t fd, short ev)
{
	if (_session->connected)
	{
		// 设置定时器回调函数 
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		event_add(&this->_timer, &tv);

		bool ret = this->_on_timer_func(this->_session);
	}
}

void SocketClient::unInit()
{

}

void SocketClient::__run()
{
	this->_base = event_base_new();
	if (NULL != this->_base)
	{
		int r = this->_session->create(_base, -1, EV_READ | EV_PERSIST, this->_session->host, this->_session->port);

		// 设置定时器
		event_assign(&this->_timer, this->_base, -1, 0, ::time_cb, this);
		//event_assign(&this->_timer, this->_base, -1, EV_PERSIST, ::time_cb, this);
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		event_add(&this->_timer, &tv);

		event_base_dispatch(this->_base);

		event_base_free(this->_base);
	}
	else
	{
		bufferevent_free(this->_session->bev);
	}
	return;
}