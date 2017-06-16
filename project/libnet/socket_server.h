#ifndef __SocketServer_H__
#define __SocketServer_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <vector>
#include <unordered_map>

struct CLIENT;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class SocketServer final
{
	typedef std::vector<CLIENT*> VTCLIENT;
	typedef std::unordered_map<lw_int32, CLIENT*> MAP_CLIENT;

public:
	SocketServer();
	~SocketServer();

public:
	lw_int32 init();
	void unInit();

public:
	lw_int32 sendData(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 run(u_short port, LW_SERVER_START_COMPLETE start_func, LW_PARSE_DATA_CALLFUNC func);
	lw_int32 getPort() { return this->_port; }

public:
	void listenerCB(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);
	void bufferreadCB(struct bufferevent *);
	void bufferwriteCB(struct bufferevent *);
	void buffereventCB(struct bufferevent *, short event);
	void timeCB(evutil_socket_t fd, short event, void *arg);

private:
	void __run();

private:
	lw_int32 _port;
	struct event_base* _base;
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
	LW_SERVER_START_COMPLETE _on_start;
	VTCLIENT vtClients;
	MAP_CLIENT mapClients;
};

#endif // !__SocketServer_H__
