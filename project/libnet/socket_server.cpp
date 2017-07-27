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
#include <event2/thread.h>

#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif // _WIN32

#include "common_marco.h"
#include "socket_session.h"
#include "socket_timer.h"

static void __listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
static void __signal_cb(evutil_socket_t, short, void *);
static void __accept_error_cb(struct evconnlistener *, void *);

static void __listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	SocketServer * server = (SocketServer*)user_data;
	server->listener_cb(listener, fd, sa, socklen);
}

static void __accept_error_cb(struct evconnlistener * listener, void * userdata)
{
	struct event_base *base = evconnlistener_get_base(listener);
	int err = EVUTIL_SOCKET_ERROR();

	printf("got an error %d (%s) on the listener. shutting down.\n", err, evutil_socket_error_to_string(err));
	
	event_base_loopexit(base, NULL);
}

static void __signal_cb(evutil_socket_t fd, short event, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 1, 0 };

	printf("caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

SocketServer::SocketServer() : _base(NULL), _onStart(NULL)
{
	_timer = new SocketTimer();
}

SocketServer::~SocketServer()
{
	if (_timer != NULL)
	{
		delete _timer;
		_timer = NULL;
	}
}

lw_int32 SocketServer::create(u_short port, ISocketServer* isession)
{
	this->_port = port;
	this->iserver = isession;

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

void SocketServer::destory()
{
	event_base_free(_base);
}

void SocketServer::listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen)
{
	struct event_base *base = evconnlistener_get_base(listener);

	SocketSession* pSession = new SocketSession(SESSION_TYPE::Server);
	int r = pSession->create(_base, fd, EV_READ | EV_WRITE, this->iserver);
	if (r == 0)
	{
		char hostBuf[NI_MAXHOST];
		char portBuf[64];
		getnameinfo(sa, socklen, hostBuf, sizeof(hostBuf), portBuf, sizeof(portBuf), NI_NUMERICHOST | NI_NUMERICSERV);

		pSession->setHost(hostBuf);
		pSession->setPort(std::stoi(portBuf));

		this->iserver->onJoin(pSession);
	}
	else
	{
		delete pSession;

		fprintf(stderr, "error constructing bufferevent!");

		event_base_loopbreak(base);
	}
}

lw_int32 SocketServer::run(LW_SERVER_START_COMPLETE func)
{
	if (NULL == func) return -1;

	if (_onStart != func)
	{
		_onStart = func;
	}

	std::thread t(std::bind(&SocketServer::__run, this));
	t.detach();

	return 0;
}

struct evconnlistener * SocketServer::createConnListener(int port)
{
	struct evconnlistener *listener = NULL;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0);	//绑定0.0.0.0地址
	sin.sin_port = htons(port);
	//inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
	listener = evconnlistener_new_bind(_base, ::__listener_cb, this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, -1, (struct sockaddr*)&sin, sizeof(sin));
	return listener;
}

void SocketServer::__run()
{
	struct event *signal_event;
	signal_event = evsignal_new(_base, SIGINT, ::__signal_cb, (void *)_base);
	if (!signal_event || event_add(signal_event, nullptr) < 0)
	{
		fprintf(stderr, "could not create/add a signal event!\n");
		return;
	}

	struct evconnlistener *listener = createConnListener(_port);
	if (listener != NULL)
	{
		evconnlistener_set_error_cb(listener, __accept_error_cb);

		// 初始化完成定时器
		{
			_timer->create(this->_base);
			_timer->startTimer(100, 2, [this](int id) -> bool
			{
				if (_onStart != NULL)
				{
					_onStart(0);
				}

				return false;
			});
		}

		int r = event_base_dispatch(_base);

		if (signal_event != NULL)
		{
			event_free(signal_event);
		}

		if (listener != NULL)
		{
			evconnlistener_free(listener);
		}
	}	

	event_base_free(_base);

	return;
}