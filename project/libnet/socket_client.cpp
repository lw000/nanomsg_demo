#include "socket_client.h"

#include <stdio.h>
#include <thread>
#include <assert.h>

#include "event2/event.h"

#include "socket_core.h"
#include "socket_processor.h"
#include "socket_session.h"

using namespace LW;

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient() : _processor(nullptr), _session(nullptr)
{

}

SocketClient::~SocketClient()
{
	delete this->_session;
	this->_session = nullptr;
}

bool SocketClient::create(SocketProcessor* processor, ISocketSessionHanlder* handler)
{													  
	this->_processor = processor;
	bool r = this->_processor->create(false);
	if (r)
	{
		this->_session = new SocketSession(handler);
		return true;
	}

	return false;
}

void SocketClient::destroy()
{
	if (this->_session != nullptr)
	{
		this->_session->destroy();
	}

	if (this->_processor != nullptr)
	{
		this->_processor->destroy();
	}
}

std::string SocketClient::debug()
{
	char buf[512];
	sprintf(buf, "%s", _session->debug());
	return std::string(buf);
}

int SocketClient::run(const std::string& addr, int port)
{
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

void SocketClient::__run()
{
	int r = this->_session->create(SESSION_TYPE::Client, _processor, -1, EV_READ | EV_PERSIST);

	if (r == 0)
	{
		this->_processor->dispatch();
	}

	this->destroy();

	return;
}