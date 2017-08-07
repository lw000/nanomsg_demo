#include "socket_client.h"

#include <stdio.h>
#include <thread>
#include <assert.h>

#include "event2/event.h"

#include "socket_core.h"

using namespace LW;

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient() : isession(nullptr)
{
	_session = new SocketSession();
	_timer = new SocketTimer();
}

SocketClient::~SocketClient()
{
	delete _session;
	_session = nullptr;

	delete _timer;
	_timer = nullptr;
}

bool SocketClient::create(ISocketSessionHanlder* isession)
{	
	if (this->openEvent(false))
	{
		_timer->create(this->_base);
	}

	this->isession = isession;

	return true;
}

void SocketClient::destroy()
{
	if (_timer != nullptr)
	{
		_timer->destroy();
	}

	if (_session != nullptr)
	{
		_session->destroy();
	}

	this->closeEvent();
}

int SocketClient::run(const char* addr, int port)
{
	if (addr == nullptr) return -1;

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

SocketTimer* SocketClient::getTimer()
{
	return this->_timer;
}

void SocketClient::__run()
{
	int r = this->_session->create(SESSION_TYPE::Client, _base, -1, EV_READ | EV_PERSIST, this->isession);

	if (r == 0)
	{
		this->dispatch();

		this->_session->destroy();
	}
	
	this->destroy();

	return;
}