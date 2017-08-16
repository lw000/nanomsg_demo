#include "socket_session.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "socket_core.h"
#include "socket_processor.h"

class CoreSocket
{
public:
	static void __read_cb(struct bufferevent* bev, void* userdata)
	{
		SocketSession *session = (SocketSession*)userdata;
		session->onRead();
	}

	static void __write_cb(struct bufferevent *bev, void *userdata)
	{
		SocketSession * session = (SocketSession*)userdata;
		session->onWrite();
	}

	static void __event_cb(struct bufferevent *bev, short event, void *userdata)
	{
		SocketSession *session = (SocketSession*)userdata;
		session->onEvent(event);
	}

	static void __on_parse_cb(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
	{
		SocketSession *session = (SocketSession *)userdata;
		session->onParse(cmd, buf, bufsize);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
SocketSession::SocketSession(ISocketSessionHanlder* handler) : _handler(handler)
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

int SocketSession::create(SESSION_TYPE c, SocketProcessor* processor, evutil_socket_t fd, short ev)
{
	this->_c = c;
	this->_processor = processor;

	switch (this->_c)
	{
	case SESSION_TYPE::Server:
	{
		this->_bev = bufferevent_socket_new(this->_processor->getBase(), fd, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->_bev, CoreSocket::__read_cb, CoreSocket::__write_cb, CoreSocket::__event_cb, this);
		bufferevent_enable(this->_bev, ev);
		this->_connected = true;
	} break;
	case SESSION_TYPE::Client:
	{
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(this->_host.c_str());
		saddr.sin_port = htons(this->_port);

		this->_bev = bufferevent_socket_new(this->_processor->getBase(), fd, BEV_OPT_CLOSE_ON_FREE);
		int con = bufferevent_socket_connect(this->_bev, (struct sockaddr *)&saddr, sizeof(saddr));
		if (con >= 0)
		{
			bufferevent_setcb(this->_bev, CoreSocket::__read_cb, NULL, CoreSocket::__event_cb, this);
			
// 			struct timeval timeout_read;
// 			evutil_timerclear(&timeout_read);
// 			timeout_read.tv_sec = 1;
// 			timeout_read.tv_usec = 0;
// 			bufferevent_set_timeouts(this->_bev, &timeout_read, NULL);

			bufferevent_enable(this->_bev, ev);
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
	this->_handler = nullptr;
	this->_host.clear();
	this->_port = -1;
	this->_c = SESSION_TYPE::NONE;
}

void SocketSession::setConnTimeout(int s)
{

}

void SocketSession::setRecvTimeout(int s)
{

}

void SocketSession::setSendTimeout(int s)
{

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

std::string SocketSession::debug()
{
	char buf[512];
	sprintf(buf, "(fd: [%d] ip: %s, port: %d, c: %d, connected:%d)", this->getSocket(), this->_host.c_str(), this->_port, this->_c, this->_connected);
	return std::string(buf);
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	if (this->_connected)
	{
		int c = this->_processor->getSocketCore()->send(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
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
		int c = this->_processor->getSocketCore()->send(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
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
		char read_buf[1024+1];

		while (input_len > 0)
		{
			size_t read_len = bufferevent_read(this->_bev, read_buf, 1024);
			if (read_len > 0)
			{
				read_buf[read_len] = '0';
				if (this->_processor->getSocketCore()->parse(read_buf, read_len, CoreSocket::__on_parse_cb, this) == 0)
				{

				}
			}
			input_len -= read_len;
		}
		/*char *read_buf = (char*)malloc(input_len);
		size_t read_len = bufferevent_read(this->_bev, read_buf, input_len);
		if (input_len == read_len)
		{
		if (this->_processor->getSocketCore()->parse(read_buf, read_len, CoreSocket::__on_parse_cb, this) == 0)
		{

		}
		}
		free(read_buf);*/
	}
}

void SocketSession::onParse(lw_int32 cmd, char* buf, lw_int32 bufsize)
{
	SocketCallback cb = nullptr;
	{
		if (!_cmd_event_map.empty())
		{
			cb = this->_cmd_event_map.at(cmd);
		}
	}
	
	bool goon = true;
	if (cb != nullptr)
	{
		goon = cb(buf, bufsize);
	}

	if (goon)
	{
		this->_handler->onSocketParse(this, cmd, buf, bufsize);
	}
}

void SocketSession::onEvent(short ev)
{
	if (ev & BEV_EVENT_CONNECTED)
	{
		this->_connected = true;
		this->_handler->onSocketConnected(this);
		return;
	}

	if (ev & BEV_EVENT_READING)
	{
		this->_handler->onSocketError(this);
	}
	else if (ev & BEV_EVENT_WRITING)
	{
		this->_handler->onSocketError(this);
	}
	else if (ev & BEV_EVENT_EOF)
	{
		this->_handler->onSocketDisConnect(this);
	}
	else if (ev & BEV_EVENT_TIMEOUT)
	{
		this->_handler->onSocketTimeout(this);
	}
	else if (ev & BEV_EVENT_ERROR)
	{
		this->_handler->onSocketError(this);
	}

	this->_connected = false;
	this->_bev = nullptr;
}