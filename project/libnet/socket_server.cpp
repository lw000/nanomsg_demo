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
#include "socket_core.h"
#include "socket_processor.h"

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

SocketServer::SocketServer() : _onFunc(nullptr)
{
	this->_core = new SocketCore;
	this->_timer = new Timer();
}

SocketServer::~SocketServer()
{
	SAFE_DELETE(this->_timer);
	SAFE_DELETE(this->_core);
}

bool SocketServer::create(SocketProcessor* processor, ISocketServerHandler* handler)
{
	this->_processor = processor;
	this->_handler = handler;

	bool r = this->_processor->create(true, this->_core);
	if (r)
	{
		this->_timer->create(this->_processor);
	}
	return r;
}

void SocketServer::destroy()
{
	if (_timer != nullptr)
	{
		this->_timer->destroy();
	}

	if (this->_processor != nullptr)
	{
		this->_processor->destroy();
	}
}

std::string SocketServer::debug()
{
	return std::string("SocketServer");
}

void SocketServer::listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen)
{
	//struct event_base *base = evconnlistener_get_base(listener);

	SocketSession* pSession = new SocketSession(this->_handler);
	int r = pSession->create(SESSION_TYPE::Server, _processor, fd, EV_READ | EV_WRITE);
	if (r == 0)
	{
		{
			char hostBuf[NI_MAXHOST];
			char portBuf[64];
			getnameinfo(sa, socklen, hostBuf, sizeof(hostBuf), portBuf, sizeof(portBuf), NI_NUMERICHOST | NI_NUMERICSERV);

			pSession->setHost(hostBuf);
			pSession->setPort(std::stoi(portBuf));
		}
		this->_handler->onListener(pSession);
	}
	else
	{
		SAFE_DELETE(pSession);
		_processor->loopbreak();
		fprintf(stderr, "error constructing SocketSession!");
	}
}

void SocketServer::listener_error_cb(struct evconnlistener * listener)
{
	struct event_base *base = evconnlistener_get_base(listener);

	int err = EVUTIL_SOCKET_ERROR();

	printf("got an error %d (%s) on the listener. shutting down.\n", err, evutil_socket_error_to_string(err));

	_processor->loopexit();
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
	listener = evconnlistener_new_bind(_processor->getBase(), ::__listener_cb, this,
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

		(ISocketThread*)(_handler)->onStart();

		int r = _processor->dispatch();

		(ISocketThread*)(_handler)->onEnd();

		if (listener != nullptr)
		{
			evconnlistener_free(listener);
		}
	}	

	this->destroy();

	return;
}

int SocketServer::loopbreak()
{
	return this->_processor->loopbreak();
}

int SocketServer::loopexit()
{
	return this->_processor->loopexit();
}