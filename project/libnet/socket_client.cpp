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

bool SocketClient::create(SocketProcessor* processor, ISocketSessionHanlder* isession)
{	
	this->_processor = processor;
	bool r = _processor->open(false);
	if (r)
	{
		this->_session = new SocketSession(isession);
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

	this->_processor->close();
}

std::string SocketClient::debug()
{
	return std::string("SocketClient");
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