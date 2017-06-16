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

using namespace LW;

#define BUF_SIZE	1024

static void read_cb(struct bufferevent* bev, void* arg);
static void event_cb(struct bufferevent *bev, short event, void *arg);
static void time_cb(evutil_socket_t fd, short event, void *arg);

static void time_cb(evutil_socket_t fd, short event, void *arg)
{
	SocketClient *client = (SocketClient*)arg;
	client->time_cb(fd, event);
}

static void read_cb(struct bufferevent* bev, void* arg)
{
	SocketClient *client = (SocketClient*)arg;
	client->read_cb(bev);
}

static void event_cb(struct bufferevent *bev, short event, void *arg)
{
	SocketClient *client = (SocketClient*)arg;
	client->event_cb(bev, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketClient::SocketClient()
{
}

SocketClient::~SocketClient()
{
}


bool SocketClient::init()
{
	return true;
}

int SocketClient::start(const char* addr, int port)
{
	if (addr == NULL) return -1;

	this->_addr = addr;
	this->_port = port;

	std::thread t(std::bind(&SocketClient::__run, this));
	t.detach();

	return 0;
}

int SocketClient::setRecvHook(LW_PARSE_DATA_CALLFUNC func)
{
	if (func == NULL) return -1;
	if (this->_on_recv_func != func)
	{
		this->_on_recv_func = func;
	}

	return 0;
}

int SocketClient::setTimerHook(std::function<bool(struct bufferevent* bev)> func)
{
 	if (func == NULL) return -1;

	this->_on_timer_func = func;

	return 0;
}

lw_int32 SocketClient::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	lw_send_socket_data(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
	{
		int c = bufferevent_write(this->_bev, p->buf, p->size);

		return c;
	});

	return -1;
}

void SocketClient::read_cb(struct bufferevent* bev)
{
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);

	{
		char *read_buf = (char*)malloc(input_len);

		size_t read_len = bufferevent_read(bev, read_buf, input_len);

		if (lw_parse_socket_data(read_buf, read_len, this->_on_recv_func, bev) == 0)
		{

		}

		free(read_buf);
	}
}

void SocketClient::time_cb(evutil_socket_t fd, short ev)
{
	if (connected)
	{
		// 设置定时器回调函数 
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		event_add(&this->_timer, &tv);

		bool ret = this->_on_timer_func(this->_bev);
	}
}

void SocketClient::event_cb(struct bufferevent *bev, short ev)
{
	this->connected = false;

	if (ev & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (ev & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	else if (ev & BEV_EVENT_TIMEOUT)
	{
		printf("timeout ...\n");
	}
	else if (ev & BEV_EVENT_CONNECTED)
	{
		this->connected = true;
		return;
	}

	//这将自动close套接字和free读写缓冲区  
	bufferevent_free(bev);
}

void SocketClient::unInit()
{

}

void SocketClient::__run()
{
	this->_base = event_base_new();
	if (NULL != this->_base)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(this->_port);
		addr.sin_addr.s_addr = inet_addr(this->_addr.c_str());
		this->_bev = bufferevent_socket_new(this->_base, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(this->_bev, ::read_cb, NULL, ::event_cb, this);
		int con = bufferevent_socket_connect(this->_bev, (struct sockaddr *)&addr, sizeof(addr));
		if (con >= 0)
		{
			bufferevent_enable(this->_bev, EV_READ | EV_PERSIST);

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
			bufferevent_free(this->_bev);
		}
	}
	return;
}