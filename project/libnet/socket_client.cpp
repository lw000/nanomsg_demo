#include "socket_client.h"

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
#include "socket_session.h"
#include "socket_timer.h"

using namespace LW;

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient()
{
	_session = new SocketSession(SocketSession::TYPE::Client);
	_timer = new SocketTimer;
}

SocketClient::~SocketClient()
{
	delete _session;
	_session = NULL;

	delete _timer;
	_timer = NULL;
}

bool SocketClient::create()
{
	if (this->_base == NULL)
	{
		this->_base = event_base_new();

		_timer->create(this->_base);
	}

	return true;
}

void SocketClient::destory()
{
	if (_timer != NULL)
	{
		_timer->destory();
	}

	if (_session != NULL)
	{
		_session->destory();
	}

	if (this->_base != NULL)
	{
		event_base_free(this->_base);
		this->_base = NULL;
	}
}

int SocketClient::run(const char* addr, int port)
{
	if (addr == NULL) return -1;

	this->_session->setHost(addr);
	this->_session->setPort(port);

	std::thread t(std::bind(&SocketClient::__run, this));
	t.detach();

	return 0;
}

SocketSession* SocketClient::getSession()
{
	return this->_session;
}

int SocketClient::setRecvHook(LW_PARSE_DATA_CALLFUNC func, void* userdata)
{
	this->_session->setRecvCall(func);
	this->_session->setUserData(userdata);
	return 0;
}

int SocketClient::startTimer(int id, int t, CLIENT_TIMERCALL func)
{
	_timer->startTimer(id, t, [this, func](int id) -> bool
	{
		return func(id, this->_session);
	});

	return 0;
}

void SocketClient::killTimer(int id)
{
	_timer->killTimer(id);
}

void SocketClient::__run()
{
	this->create();
	if (NULL != this->_base)
	{
		int r = this->_session->create(_base, -1, EV_READ | EV_PERSIST);

		event_base_dispatch(this->_base);

		this->destory();
	}
	else
	{
		this->_session->destory();
	}
	return;
}