#include "socket_session.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

static void __read_cb(struct bufferevent* bev, void* arg)
{
	SocketSession *session = (SocketSession*)arg;
	session->read_ev();
}

static void __write_cb(struct bufferevent *bev, void *arg)
{
	SocketSession * session = (SocketSession*)arg;
	session->write_ev();
}

static void __event_cb(struct bufferevent *bev, short event, void *arg)
{
	SocketSession *session = (SocketSession*)arg;
	session->event_ev(event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SocketSession::SocketSession(TYPE c)
	: _bev(NULL)
	, _on_recv_func(NULL)
	, _connected(false)
	, _port(-1)
{
	this->_c = c;
}

SocketSession::~SocketSession()
{
	this->destory();
}

int SocketSession::setRecvCall(LW_PARSE_DATA_CALLFUNC func)
{
	if (func == NULL) return -1;
	if (_on_recv_func != func)
	{
		_on_recv_func = func;
	}

	return 0;
}

int SocketSession::create(struct event_base* base, evutil_socket_t fd, short event)
{
	switch (_c)
	{
	case SocketSession::Client:
	{
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(this->_host.c_str());
		saddr.sin_port = htons(this->_port);

		this->_bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->_bev, ::__read_cb, NULL, ::__event_cb, this);
		int con = bufferevent_socket_connect(this->_bev, (struct sockaddr *)&saddr, sizeof(saddr));
		if (con >= 0)
		{
			bufferevent_enable(this->_bev, event);
			_connected = true;
		}
		else
		{
			_connected = false;
		}
	} break;
	case SocketSession::Server:
	{
		this->_bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->_bev, ::__read_cb, ::__write_cb, ::__event_cb, this);
		bufferevent_enable(this->_bev, EV_READ | EV_WRITE);
		_connected = true;
	} break;
	default:
		break;
	}

	return 0;
}

void SocketSession::destory()
{
	if (this->_bev != NULL)
	{
		bufferevent_free(this->_bev);
	}
	this->_bev = NULL;
}

void SocketSession::setHost(std::string host)
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
	return _port;
}

bool SocketSession::connected()
{ 
	return _connected;
}

evutil_socket_t SocketSession::getSocket() 
{ 
	return bufferevent_getfd(this->_bev);
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	if (_connected)
	{
		int c = lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
		{
			int c = bufferevent_write(_bev, p->buf, p->size);

			return c;
		});
		return c;
	}

	return 0;
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize, SocketCallback cb)
{
	if (_connected)
	{
		int c = lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
		{
			int c = bufferevent_write(_bev, p->buf, p->size);

			return c;
		});
		return c;
	}

	return 0;
}

void SocketSession::write_ev()
{

}

void SocketSession::read_ev()
{
	struct evbuffer *input = bufferevent_get_input(_bev);
	size_t input_len = evbuffer_get_length(input);

	{
		char *read_buf = (char*)malloc(input_len);
		size_t read_len = bufferevent_read(_bev, read_buf, input_len);
		if (input_len == read_len)
		{
			if (lw_parse_socket_data(read_buf, read_len, this->_on_recv_func, this) == 0)
			{

			}
		}
		free(read_buf);
	}
}

void SocketSession::event_ev(short ev)
{
	_connected = false;

	evutil_socket_t fd = bufferevent_getfd(_bev);

	if (ev & BEV_EVENT_READING)
	{
		printf("[%d]: EVENT_READING\n", fd);
	}
	else if (ev & BEV_EVENT_WRITING)
	{
		printf("[%d]: BEV_EVENT_WRITING\n", fd);
	}
	else if (ev & BEV_EVENT_EOF)
	{
		printf("[%d]: connection closed.\n", fd);
	}
	else if (ev & BEV_EVENT_TIMEOUT)
	{
		printf("[%d]: BEV_EVENT_TIMEOUT.\n", fd);
	}
	else if (ev & BEV_EVENT_ERROR)
	{
		
	}
	else if (ev & BEV_EVENT_CONNECTED)
	{
		_connected = true;
		return;
	}

	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(_bev);
	this->_bev = NULL;
}