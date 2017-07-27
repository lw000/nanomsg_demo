#ifndef __SocketServer_H__
#define __SocketServer_H__

#include <vector>
#include <list>
#include <unordered_map>
#include <event2/util.h>

#include "business.h"
#include "object.h"
#include "socket_session.h"

class SocketSession;
class SocketTimer;

struct event_base;
struct evconnlistener;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class ISocketServer : public ISocketSession
{
public:
	virtual ~ISocketServer() {}

public:
	virtual void onJoin(SocketSession* session) = 0;
};

class SocketServer : public Object
{
public:
	SocketServer();
	virtual ~SocketServer();

public:
	lw_int32 create(u_short port, ISocketServer* isession);
	lw_int32 run(LW_SERVER_START_COMPLETE start_func);
	void destory();

public:
	lw_int32 getPort() const { return this->_port; }

public:
	void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);

private:
	struct evconnlistener * createConnListener(int port);

private:
	void __run();

private:
	lw_int32 _port;
	struct event_base* _base;
	SocketTimer* _timer;

private:
	LW_SERVER_START_COMPLETE _onStart;
	ISocketServer* iserver;
};

#endif // !__SocketServer_H__
