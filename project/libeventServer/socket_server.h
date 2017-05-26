#ifndef __Server_H__
#define __Server_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <vector>

struct CLIENT;

typedef void(*LW_SERVER_START_COMPLETE)(lw_int32 what);

class SocketServer final
{
	typedef std::vector<CLIENT*> VTCLIENT;

public:
	SocketServer();
	~SocketServer();

public:
	lw_int32 init();
	void unInit();

public:
	lw_int32 sendData(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 run(u_short port, LW_SERVER_START_COMPLETE start_func, LW_PARSE_DATA_CALLFUNC func);

public:
	void listenerCB(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
	void bufferreadCB(struct bufferevent *, void *);
	void bufferwriteCB(struct bufferevent *, void *);
	void buffereventCB(struct bufferevent *, short, void *);
	void timeCB(evutil_socket_t fd, short event, void *arg);
private:
	struct event_base* _base;
	
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
	LW_SERVER_START_COMPLETE _on_start;
	VTCLIENT vtClients;
};

#endif // !__Server_H__
