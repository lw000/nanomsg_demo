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
static void __listener_error_cb(struct evconnlistener *, void *);

static void __listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *userdata)
{
	SocketServer * server = (SocketServer*)userdata;
	server->listener_cb(listener, fd, sa, socklen);
}

static void __listener_error_cb(struct evconnlistener * listener, void * userdata)
{
	SocketServer * server = (SocketServer*)userdata;
	server->listener_error_cb(listener);
}

SocketServer::SocketServer(EventObject* evObject, ISocketServerHandler* isession) : _onFunc(nullptr)
{
	this->_timer = new Timer();
	_evObject = evObject;
	this->iserver = isession;
}

SocketServer::~SocketServer()
{
	if (_timer != nullptr)
	{
		delete this->_timer;
		this->_timer = nullptr;
	}
}

bool SocketServer::create()
{
	bool r = _evObject->openServer();
	if (r)
	{
		this->_timer->create(_evObject);
	}
	return r;
}

void SocketServer::destroy()
{
	if (_timer != nullptr)
	{
		_timer->destroy();
	}

	_evObject->close();
}

std::string SocketServer::debug()
{
	return std::string("SocketServer");
}

void SocketServer::listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen)
{
	//struct event_base *base = evconnlistener_get_base(listener);

	SocketSession* pSession = new SocketSession();
	int r = pSession->create(SESSION_TYPE::Server, _evObject, fd, EV_READ | EV_WRITE, this->iserver);
	if (r == 0)
	{
		char hostBuf[NI_MAXHOST];
		char portBuf[64];
		getnameinfo(sa, socklen, hostBuf, sizeof(hostBuf), portBuf, sizeof(portBuf), NI_NUMERICHOST | NI_NUMERICSERV);

		pSession->setHost(hostBuf);
		pSession->setPort(std::stoi(portBuf));

		this->iserver->onListener(pSession);
	}
	else
	{
		delete pSession;
		_evObject->loopbreak();
		fprintf(stderr, "error constructing SocketSession!");
	}
}

void SocketServer::listener_error_cb(struct evconnlistener * listener)
{
	struct event_base *base = evconnlistener_get_base(listener);

	int err = EVUTIL_SOCKET_ERROR();

	printf("got an error %d (%s) on the listener. shutting down.\n", err, evutil_socket_error_to_string(err));

	_evObject->loopexit();
}

lw_int32 SocketServer::run(u_short port, std::function<void(lw_int32 what)> func)
{
 	if (nullptr == func) return -1;

	this->_port = port;
	this->_onFunc = func;

	std::thread t(std::bind(&SocketServer::__run, this));
	t.detach();

	return 0;
}

struct evconnlistener * SocketServer::__createConnListener(int port)
{
	struct evconnlistener *listener = nullptr;
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(0);	//绑定0.0.0.0地址
	sin.sin_port = htons(port);
	//inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
	listener = evconnlistener_new_bind(_evObject->getBase(), ::__listener_cb, this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, -1, (struct sockaddr*)&sin, sizeof(sin));
	return listener;
}

void SocketServer::__run()
{
	struct evconnlistener *listener = __createConnListener(this->_port);
	if (listener != nullptr)
	{
		evconnlistener_set_error_cb(listener, __listener_error_cb);

		// 初始化完成定时器
		{
			this->_timer->start(100, 1000, [this](int tid, unsigned int tms) -> bool
			{
				if (this->_onFunc != nullptr)
				{
					this->_onFunc(0);
				}

				return false;
			});
		}

		int r = _evObject->dispatch();

		if (listener != nullptr)
		{
			evconnlistener_free(listener);
		}
	}	

	this->destroy();

	return;
}