#include "socket_session.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "socket_processor.h"

class CoreSocket
{
public:
	static void __read_cb(struct bufferevent* bev, void* arg)
	{
		SocketSession *session = (SocketSession*)arg;
		session->onRead();
	}

	static void __write_cb(struct bufferevent *bev, void *arg)
	{
		SocketSession * session = (SocketSession*)arg;
		session->onWrite();
	}

	static void __event_cb(struct bufferevent *bev, short event, void *arg)
	{
		SocketSession *session = (SocketSession*)arg;
		session->onEvent(event);
	}

	static void __on_parse_cb(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
	{
		SocketSession *session = (SocketSession *)userdata;
		session->onSocketParse(session, cmd, buf, bufsize);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
SocketSession::SocketSession(ISocketSessionHanlder* isession) : _isession(isession)
{
	this->_connected = false;
	this->_bev = nullptr;
	this->_host.clear();
	this->_port = -1;
	this->_c = SESSION_TYPE::NONE;
}

SocketSession::~SocketSession()
{
	this->reset();
}

int SocketSession::create(SESSION_TYPE c, SocketProcessor* base, evutil_socket_t fd, short event)
{
	this->_c = c;

	switch (this->_c)
	{
	case SESSION_TYPE::Server:
	{
		this->_bev = bufferevent_socket_new(base->getBase(), fd, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->_bev, CoreSocket::__read_cb, CoreSocket::__write_cb, CoreSocket::__event_cb, this);
		bufferevent_enable(this->_bev, EV_READ | EV_WRITE);
		_connected = true;
	} break;
	case SESSION_TYPE::Client:
	{
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(this->_host.c_str());
		saddr.sin_port = htons(this->_port);

		this->_bev = bufferevent_socket_new(base->getBase(), fd, BEV_OPT_CLOSE_ON_FREE);
		int con = bufferevent_socket_connect(this->_bev, (struct sockaddr *)&saddr, sizeof(saddr));
		if (con >= 0)
		{
			bufferevent_setcb(this->_bev, CoreSocket::__read_cb, NULL, CoreSocket::__event_cb, this);
			bufferevent_enable(this->_bev, event);
			this->_connected = true;
		}
		else
		{
			this->_connected = false;
		}
	} break;	
	default:
		break;
	}

	return 0;
}

void SocketSession::destroy()
{
	this->reset();
}

void SocketSession::reset()
{
	this->_connected = false;
	this->_bev = nullptr;
	this->_isession = nullptr;
	this->_host.clear();
	this->_port = -1;
	this->_c = SESSION_TYPE::NONE;
}

void SocketSession::setHost(const std::string& host)
{
	this->_host = host;
}

std::string SocketSession::getHost()
{
	return this->_host;
}

void SocketSession::setPort(int port)
{
	this->_port = port;
}

int SocketSession::getPort()
{
	return this->_port;
}

bool SocketSession::connected()
{ 
	return this->_connected;
}

evutil_socket_t SocketSession::getSocket() 
{ 
	evutil_socket_t fd = bufferevent_getfd(this->_bev);
	return fd;
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	if (this->_connected)
	{
		int c = lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
		{
			int c = bufferevent_write(this->_bev, p->buf, p->size);

			return c;
		});

		return c;
	}

	return -1;
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize, lw_int32 recvcmd, SocketCallback cb)
{
	if (this->_connected)
	{
		int c = lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
		{
			int c1 = bufferevent_write(this->_bev, p->buf, p->size);
			return c1;
		});

		{
			this->_cmd_event_map[recvcmd] = cb;
		}

		return c;
	}

	return -1;
}

void SocketSession::onWrite()
{

}

void SocketSession::onRead()
{
	struct evbuffer *input = bufferevent_get_input(this->_bev);
	size_t input_len = evbuffer_get_length(input);

	{
		char *read_buf = (char*)malloc(input_len);
		size_t read_len = bufferevent_read(this->_bev, read_buf, input_len);
		if (input_len == read_len)
		{
			if (lw_parse_socket_data(read_buf, read_len, CoreSocket::__on_parse_cb, this) == 0)
			{

			}
		}
		free(read_buf);
	}
}

int SocketSession::onSocketConnected(SocketSession* session)
{
	this->_connected = true;
	this->_isession->onSocketConnected(this);
	return 0;
}

int SocketSession::onSocketDisConnect(SocketSession* session)
{
	this->_isession->onSocketDisConnect(this);
	return 0;
}

int SocketSession::onSocketTimeout(SocketSession* session)
{
	this->_isession->onSocketTimeout(this);
	return 0;
}

int SocketSession::onSocketError(SocketSession* session)
{
	this->_isession->onSocketError(this);
	return 0;
}

void SocketSession::onSocketParse(SocketSession* session, lw_int32 cmd, char* buf, lw_int32 bufsize)
{
	SocketCallback cb = nullptr;
	{
		cb = this->_cmd_event_map[cmd];
	}
	
	bool goon = true;
	if (cb != nullptr)
	{
		goon = cb(buf, bufsize);
	}

	if (goon)
	{
		this->_isession->onSocketParse(this, cmd, buf, bufsize);
	}
}

void SocketSession::onEvent(short ev)
{
	if (ev & BEV_EVENT_CONNECTED)
	{
		_connected = true;
		this->_isession->onSocketConnected(this);
		return;
	}

	if (ev & BEV_EVENT_READING)
	{
		this->onSocketError(this);
	}
	else if (ev & BEV_EVENT_WRITING)
	{
		this->onSocketError(this);
	}
	else if (ev & BEV_EVENT_EOF)
	{
		this->onSocketDisConnect(this);
	}
	else if (ev & BEV_EVENT_TIMEOUT)
	{
		this->onSocketTimeout(this);
	}
	else if (ev & BEV_EVENT_ERROR)
	{
		this->onSocketError(this);
	}

	this->_connected = false;
	this->_bev = nullptr;
}