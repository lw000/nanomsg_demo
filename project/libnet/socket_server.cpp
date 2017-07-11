#include "socket_server.h"

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

#include "common_marco.h"

#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif // _WIN32

#include "session.h"

static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
static void read_cb(struct bufferevent *, void *);
static void write_cb(struct bufferevent *, void *);
static void event_cb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);
static void time_cb(evutil_socket_t fd, short event, void *arg);

static void time_cb(evutil_socket_t fd, short event, void *arg)
{
	SocketServer * sev = (SocketServer*)arg;
	sev->timeCB(fd, event, arg);
}

static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	SocketServer * server = (SocketServer*)user_data;
	server->listenerCB(listener, fd, sa, socklen);
}

// static void read_cb(struct bufferevent *bev, void *user_data)
// {
// 	SocketServer * server = (SocketServer*)user_data;
// 	server->readCB(bev);
// }
// 
// static void write_cb(struct bufferevent *bev, void *user_data)
// {
// 	SocketServer * server = (SocketServer*)user_data;
// 	server->writeCB(bev);
// }
// 
// static void event_cb(struct bufferevent *bev, short event, void *user_data)
// {
// 	SocketServer * server = (SocketServer*)user_data;
// 	server->eventCB(bev, event);
// }

static void accept_error_cb(struct evconnlistener * listener, void * userdata)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();

	printf("got an error %d (%s) on the listener. shutting down.\n", err, evutil_socket_error_to_string(err));
	
	event_base_loopexit(base, NULL);
}

static void signal_cb(evutil_socket_t fd, short event, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 1, 0 };

	printf("caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

SocketServer::SocketServer() : _base(NULL), _on_start(NULL), _on_recv_func(NULL)
{
}

SocketServer::~SocketServer()
{
	SESSIONS::iterator iter = sessions.begin();
	for (iter; iter != sessions.end(); ++iter)
	{
		delete iter->second;
	}
}

lw_int32 SocketServer::init()
{
#ifdef WIN32
	struct event_config *cfg = event_config_new();
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg)
	{
		_base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}
#else
	_base = event_base_new();
#endif
	
	if (!_base) return -1;

	return 0;
}

void SocketServer::unInit()
{
	event_base_free(_base);
}

lw_int32 SocketServer::sendData(SocketSession* session, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	lw_int32 result = 0;
	{
		result = session->sendData(cmd, object, objectSize);
	}
	return result;
}

void SocketServer::timeCB(evutil_socket_t fd, short event, void *arg)
{
	if (_on_start != NULL)
	{
		_on_start(0);
	}
}

void SocketServer::writeCB(struct bufferevent *bev)
{
	evutil_socket_t fd = bufferevent_getfd(bev);
	SocketSession* psesion = sessions[fd];

	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0)
	{

	}
}

// 从客户端读取数据
void SocketServer::readCB(struct bufferevent *bev)
{
	evutil_socket_t fd = bufferevent_getfd(bev);
	
	SocketSession* psesion = sessions[fd];

	struct evbuffer *input;
	input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);

	lw_char8 *read_buf = (lw_char8*)malloc(input_len);

	size_t read_len = bufferevent_read(bev, read_buf, input_len);

	if (read_len == input_len)
	{
		if (lw_parse_socket_data(read_buf, read_len, this->_on_recv_func, bev) == 0)
		{

		}
	}

	free(read_buf);
}

void SocketServer::eventCB(struct bufferevent *bev, short event)
{
	evutil_socket_t fd = bufferevent_getfd(bev);
	
	if (event & BEV_EVENT_READING)
	{
		printf("[%d]: EVENT_READING\n", fd);
	}
	else if (event & BEV_EVENT_WRITING)
	{
		printf("[%d]: BEV_EVENT_WRITING\n", fd);
	}
	else if (event & BEV_EVENT_EOF)
	{
		printf("[%d]: connection closed.\n", fd);
	}
	else if (event & BEV_EVENT_TIMEOUT)
	{
		printf("[%d]: BEV_EVENT_TIMEOUT.\n", fd);
	}
	else if (event & BEV_EVENT_ERROR)
	{
		SESSIONS::iterator iter = sessions.begin();
		for (iter; iter != sessions.end(); ++iter)
		{
			if (iter->first == fd)
			{
				printf("leave ([%d] host=%s, port=%d)\n", fd, iter->second->host.c_str(), iter->second->port);
				delete iter->second;
				sessions.erase(iter);
				break;
			}
		}
	}

	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(bev);
}

void SocketServer::listenerCB(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen)
{
	struct event_base *base = evconnlistener_get_base(listener);

	SocketSession* pSession = new SocketSession(SocketSession::TYPE::Server);
	int r = pSession->create(_base, fd, EV_READ | EV_WRITE);
	pSession->setRecvCall(_on_recv_func);

	if (r == 0)
	{
		char hbuf[NI_MAXHOST];
		lw_uint32 sbuf;
		getnameinfo(sa, socklen, hbuf, sizeof(hbuf), (char*)&sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
		
		printf("join (host = %s, port = %d)\n", hbuf, sbuf);

		evutil_socket_t new_fd = pSession->getSocket();

		sessions[new_fd] = pSession;
	}
	else
	{
		delete pSession;

		fprintf(stderr, "error constructing bufferevent!");

		event_base_loopbreak(base);
	}
}

lw_int32 SocketServer::run(u_short port, LW_SERVER_START_COMPLETE start_func, LW_PARSE_DATA_CALLFUNC func)
{
	if (NULL == func) return -1;
	if (NULL == start_func) return -1;

	if (func != _on_recv_func)
	{
		_on_recv_func = func;
	}

	if (_on_start != start_func)
	{
		_on_start = start_func;
	}

	if (_port != port)
	{
		_port = port;
	}

	std::thread t(std::bind(&SocketServer::__run, this));
	t.detach();

	return 0;
}

void SocketServer::__run()
{
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0);	//绑定0.0.0.0地址
	sin.sin_port = htons(_port);

	struct evconnlistener *listener;
	listener = evconnlistener_new_bind(_base, ::listener_cb, this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, -1, (struct sockaddr*)&sin, sizeof(sin));

	struct event *signal_event;
	signal_event = evsignal_new(_base, SIGINT, ::signal_cb, (void *)_base);
	if (!signal_event || event_add(signal_event, nullptr) < 0)
	{
		fprintf(stderr, "could not create/add a signal event!\n");
		return;
	}

	evconnlistener_set_error_cb(listener, accept_error_cb);

	// 初始化完成定时器 
	{
		struct event evtimer;
		event_assign(&evtimer, _base, 0, 0, time_cb, this);
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		event_add(&evtimer, &tv);
	}
	
	// 客户端存活检测定时器
// 	{
// 		struct event _client_live_timer;
// 		event_assign(&_client_live_timer, _base, 0, 0, time_cb, this);
// 		struct timeval tv;
// 		evutil_timerclear(&tv);
// 		tv.tv_sec = 1;
// 		tv.tv_usec = 0;
// 		event_add(&_client_live_timer, &tv);
// 	}

	int ret = event_base_dispatch(_base);

	event_free(signal_event);
	evconnlistener_free(listener);

	event_base_free(_base);

	return;
}