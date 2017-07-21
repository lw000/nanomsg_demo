#ifndef __SocketServer_H__
#define __SocketServer_H__

#include <vector>
#include <list>
#include <unordered_map>
#include <event2/util.h>

#include "business.h"
#include "object.h"

class SocketSession;
class SocketTimer;

struct event_base;
struct evconnlistener;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class SocketServer : public Object
{
	typedef std::unordered_map<lw_int32, SocketSession*> SESSIONS;

public:
	SocketServer();
	virtual ~SocketServer();

public:
	lw_int32 create(u_short port);
	void destory();

public:
	lw_int32 sendData(SocketSession* session, lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 run(LW_SERVER_START_COMPLETE start_func, LW_PARSE_DATA_CALLFUNC func);

public:
	lw_int32 getPort() const { return this->_port; }

public:
	void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);
	void event_cb(struct bufferevent *, short event);

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
	LW_PARSE_DATA_CALLFUNC _onRecvfunc;
	LW_SERVER_START_COMPLETE _onStart;
};

#endif // !__SocketServer_H__
