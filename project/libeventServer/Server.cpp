#include "Server.h"

#include <assert.h>
#include <signal.h>
#include <thread>
#include <vector>
#include <algorithm>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>
#include "iocp-internal.h"

#include "common_marco.h"

#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif // _WIN32


static Server * __gs_shared_server = NULL;

struct CLIENT
{
	struct bufferevent* bev;
	bool live;
	char ip[64];
	char port[8];
};

static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
static void bufferread_cb(struct bufferevent *, void *);
static void bufferwrite_cb(struct bufferevent *, void *);
static void bufferevent_cb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	__gs_shared_server->listenerCB(listener, fd, sa, socklen, user_data);
}

static void bufferread_cb(struct bufferevent *bev, void *user_data)
{
	__gs_shared_server->bufferreadCB(bev, user_data);
}

static void bufferwrite_cb(struct bufferevent *bev, void *user_data)
{
	__gs_shared_server->bufferwriteCB(bev, user_data);
}

static void bufferevent_cb(struct bufferevent *bev, short event, void *user_data)
{
	__gs_shared_server->buffereventCB(bev, event, user_data);
}

static void signal_cb(evutil_socket_t fd, short event, void *user_data)
{
	__gs_shared_server->signalCB(fd, event, user_data);
}

static void log_cb(int severity, const char *msg)
{

}

static void accept_error_cb(struct evconnlistener * listener, void * userdata)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();

	printf("got an error %d (%s) on the listener. shutting down.\n", err, evutil_socket_error_to_string(err));
	
	event_base_loopexit(base, NULL);
}


Server* Server::sharedInstance()
{
	return __gs_shared_server;
}

Server::Server() : _base(NULL), _on_recv_func(NULL)
{
	__gs_shared_server = this;

}

Server::~Server()
{
	VTCLIENT::iterator iter = vtClients.begin();
	for (iter; iter != vtClients.end(); ++iter)
	{
		delete (*iter);
	}

}

lw_int32 Server::init()
{


	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数

	event_set_log_callback(log_cb);
#ifdef WIN32

	evthread_use_windows_threads();

	struct event_config *cfg = event_config_new();
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg) {
		_base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}
#else
	_base = event_base_new();
#endif
	
	const char* method = event_base_get_method(_base);
	printf("method: %s\n", method);

	if (!_base) return -1;

	return 0;
}

void Server::unInit()
{
	event_base_free(_base);
}

event_base* Server::getEventBase()
{
	return _base;
}

lw_int32 Server::sendData(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	lw_int32 result = 0;
	{
		LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);
		result = bufferevent_write(bev, p->buf, p->size);
		lw_free_net_message(p);
	}

	return result;
}

void Server::signalCB(evutil_socket_t fd, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 1, 0 };

	printf("caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

void Server::bufferwriteCB(struct bufferevent *bev, void *user_data)
{
	CLIENT* pClient = (CLIENT*)user_data;

	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0)
	{
		//printf("flushed answer \n");

		//bufferevent_free(bev);
	}
}

// 从客户端读取数据
void Server::bufferreadCB(struct bufferevent *bev, void *user_data)
{
	CLIENT* pClient = (CLIENT*)user_data;
	{
		struct evbuffer *input;
		input = bufferevent_get_input(bev);
		size_t input_len = evbuffer_get_length(input);

		char *read_buf = (char*)malloc(input_len);
		
		size_t read_len = bufferevent_read(bev, read_buf, input_len);

		if (read_len == input_len)
		{
			if (lw_parse_socket_data(read_buf, read_len, _on_recv_func, bev) == 0)
			{

			}
		}

		free(read_buf);
	}
}

void Server::buffereventCB(struct bufferevent *bev, short event, void *user_data)
{
	CLIENT* pClient = (CLIENT*)user_data;

	if (event & BEV_EVENT_READING)
	{
		printf("EVENT_READING\n");
	}
	else if (event & BEV_EVENT_WRITING)
	{
		printf("BEV_EVENT_WRITING\n");
	}
	else if (event & BEV_EVENT_EOF)
	{
		printf("connection closed.\n");
	}
	else if (event & BEV_EVENT_TIMEOUT)
	{
		printf("BEV_EVENT_TIMEOUT.\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
//		printf("got an error on the connection: %s\n", strerror(errno));/*XXX win32*/

		VTCLIENT::iterator iter = vtClients.begin();
		for (iter; iter != vtClients.end(); ++iter)
		{
			if ((*iter)->bev == bev)
			{
				printf("leave (host=%s, port=%s)\n", (*iter)->ip, (*iter)->port);
				delete (*iter);
				vtClients.erase(iter);
				break;
			}
		}
	}

	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(bev);
}

void Server::listenerCB(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;

	struct bufferevent *bev;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (NULL != bev)
	{
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		getnameinfo(sa, socklen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			NI_NUMERICHOST | NI_NUMERICSERV);
		printf("join (host=%s, port=%s)\n", hbuf, sbuf);

		CLIENT* pClient = new CLIENT();
		pClient->bev = bev;
		pClient->live = true;
		strcpy(pClient->ip, hbuf);
		strcpy(pClient->port, sbuf);

		bufferevent_setcb(bev, ::bufferread_cb, ::bufferwrite_cb, ::bufferevent_cb, pClient);
		bufferevent_enable(bev, EV_WRITE | EV_READ);

		vtClients.push_back(pClient);
	}
	else
	{
		fprintf(stderr, "error constructing bufferevent!");
		event_base_loopbreak(base);
	}
}

lw_int32 Server::run(u_short port, LW_PARSE_DATA_CALLFUNC func)
{
	if (!func) return -2;

	if (func != _on_recv_func)
	{
		_on_recv_func = func;
	}
	
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0);	//绑定0.0.0.0地址
	sin.sin_port = htons(port);

	struct evconnlistener *listener;
	listener = evconnlistener_new_bind(_base, ::listener_cb, (void *)_base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, -1, (struct sockaddr*)&sin, sizeof(sin));

	struct event *signal_event;
	signal_event = evsignal_new(_base, SIGINT, ::signal_cb, (void *)_base);
	if (!signal_event || event_add(signal_event, nullptr) < 0)
	{
		fprintf(stderr, "could not create/add a signal event!\n");
		return -3;
	}

	evconnlistener_set_error_cb(listener, accept_error_cb);

	int ret = event_base_dispatch(_base);

	event_free(signal_event);
	evconnlistener_free(listener);
	event_base_free(_base);

	return ret;
}