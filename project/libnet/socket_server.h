#ifndef __SocketServer_H__
#define __SocketServer_H__

#include "business.h"
#include <event2/util.h>

#include <vector>
#include <list>
#include <unordered_map>

class SocketSession;
class SocketTimer;
struct event_base;
struct evconnlistener;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class SocketServer final
{
	typedef std::unordered_map<lw_int32, SocketSession*> SESSIONS;

public:
	SocketServer();
	~SocketServer();

public:
	lw_int32 create(u_short port);
	void destory();

public:
	lw_int32 sendData(SocketSession* session, lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 run(LW_SERVER_START_COMPLETE start_func, LW_PARSE_DATA_CALLFUNC func);

public:
	lw_int32 getPort() { return this->_port; }

public:
	void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);
	void event_cb(struct bufferevent *, short event);
	void timer_cb(evutil_socket_t fd, short event, void *arg);

private:
	struct evconnlistener * createConnListener(int port);

private:
	void __run();

private:
	lw_int32 _port;
	struct event_base* _base;
	SocketTimer* _timer;
	SESSIONS sessions;

private:
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
	LW_SERVER_START_COMPLETE _on_start;
};

#endif // !__SocketServer_H__
