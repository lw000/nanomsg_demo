#include "socket_client.h"

#include <stdio.h>
#include <thread>
#include <assert.h>

#include "event2/event.h"

#include "socket_core.h"
#include "event_object.h"
#include "socket_session.h"

using namespace LW;

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient(EventObject* evObject, ISocketClient* isession) : _isession(nullptr)
{
	_session = new SocketSession();
	_evObject = evObject;
	this->_isession = isession;
}

SocketClient::~SocketClient()
{
	delete _session;
	_session = nullptr;
}

bool SocketClient::create()
{	
	bool r = _evObject->openClient();

	return true;
}

void SocketClient::destroy()
{
	if (_session != nullptr)
	{
		_session->destroy();
	}

	if (_isession != nullptr)
	{
		delete _isession;
		_isession = nullptr;
	}

	_evObject->close();
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
	int r = this->_session->create(SESSION_TYPE::Client, _evObject, -1, EV_READ | EV_PERSIST, this);

	if (r == 0)
	{
		_evObject->dispatch();

		this->_session->destroy();
	}
	
	this->destroy();

	return;
}


int SocketClient::onSocketConnected(SocketSession* session)
{
	this->_isession->onSocketConnected();
	return 0;
}

int SocketClient::onSocketDisConnect(SocketSession* session)
{
	this->_isession->onSocketDisConnect();
	return 0;
}

int SocketClient::onSocketTimeout(SocketSession* session)
{
	this->_isession->onSocketTimeout();
	return 0;
}

int SocketClient::onSocketError(SocketSession* session)
{
	this->_isession->onSocketError();
	return 0;
}

void SocketClient::onSocketParse(SocketSession* session, lw_int32 cmd, char* buf, lw_int32 bufsize)
{
	this->_isession->onSocketParse(cmd, buf, bufsize);
}