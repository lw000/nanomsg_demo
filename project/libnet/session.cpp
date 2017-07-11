#include "session.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

static void read_cb(struct bufferevent* bev, void* arg)
{
	SocketSession *session = (SocketSession*)arg;
	session->read_cb();
}

static void write_cb(struct bufferevent *bev, void *arg)
{
	SocketSession * session = (SocketSession*)arg;
	session->write_cb();
}

static void event_cb(struct bufferevent *bev, short event, void *arg)
{
	SocketSession *session = (SocketSession*)arg;
	session->event_cb(event);
}

SocketSession::SocketSession(TYPE c) : bev(NULL), _on_recv_func(NULL), connected(false), port(-1)
{
	this->c = c;
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

int SocketSession::create(struct event_base* base, evutil_socket_t fd, short event, std::string addr, int port)
{
	this->host = host;
	this->port = port;	
	switch (c)
	{
	case SocketSession::Client:
	{
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(this->host.c_str());
		saddr.sin_port = htons(this->port);

		this->bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->bev, ::read_cb, NULL, ::event_cb, this);
		int con = bufferevent_socket_connect(this->bev, (struct sockaddr *)&saddr, sizeof(saddr));
		if (con >= 0)
		{
			bufferevent_enable(this->bev, event);
			connected = true;
		}
		else
		{
			connected = false;
		}
	} break;
	case SocketSession::Server:
	{
		this->bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->bev, ::read_cb, ::write_cb, ::event_cb, this);
		bufferevent_enable(this->bev, EV_READ | EV_WRITE);
		connected = true;
	} break;
	default:
		break;
	}

	return 0;
}

evutil_socket_t SocketSession::getSocket() 
{ 
	return bufferevent_getfd(this->bev);
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	int c = lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
	{
		int c = bufferevent_write(bev, p->buf, p->size);

		return c;
	});

	return c;
}

void SocketSession::write_cb()
{

}

void SocketSession::read_cb()
{
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);

	{
		char *read_buf = (char*)malloc(input_len);

		size_t read_len = bufferevent_read(bev, read_buf, input_len);

		if (lw_parse_socket_data(read_buf, read_len, this->_on_recv_func, this) == 0)
		{

		}

		free(read_buf);
	}
}

void SocketSession::event_cb(short ev)
{
	connected = false;

	evutil_socket_t fd = bufferevent_getfd(bev);

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
		connected = true;
		return;
	}

	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(bev);

	//if (ev & BEV_EVENT_EOF)
	//{
	//	printf("connection closed\n");
	//}
	//else if (ev & BEV_EVENT_ERROR)
	//{
	//	printf("some other error\n");
	//}
	//else if (ev & BEV_EVENT_TIMEOUT)
	//{
	//	printf("timeout ...\n");
	//}
	//else if (ev & BEV_EVENT_CONNECTED)
	//{
	//	connected = true;
	//	return;
	//}

	////这将自动close套接字和free读写缓冲区  
	//bufferevent_free(bev);
}