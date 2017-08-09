#ifndef __SocketServer_H__
#define __SocketServer_H__

#include <vector>
#include <list>
#include <unordered_map>
#include <event2/util.h>

#include "socket_core.h"
#include "event_object.h"
#include "socket_session.h"
#include <functional>

class SocketSession;
class Timer;

struct event_base;
struct evconnlistener;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class ISocketServerHandler : public ISocketSessionHanlder
{
public:
	virtual ~ISocketServerHandler() {}

public:
	virtual void onListener(SocketSession* session) = 0;
};

class SocketServer : public Object
{
public:
	SocketServer(EventObject* evObject, ISocketServerHandler* isession);
	virtual ~SocketServer();

public:
	bool create();
	void destroy();

public:
	lw_int32 run(u_short port, std::function<void(lw_int32 what)> func);

public:
	int getPort() const { return this->_port; }

public:
	void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);
	void listener_error_cb(struct evconnlistener *);

public:
	virtual std::string debug() override;

private:
	struct evconnlistener * __createConnListener(int port);

private:
	void __run();

private:
	EventObject* _evObject;
	lw_int32 _port;
	Timer* _timer;

private:
	std::function<void(lw_int32 what)> _onFunc;
	ISocketServerHandler* iserver;
};

#endif // !__SocketServer_H__ 
