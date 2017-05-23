#ifndef __Server_H__
#define __Server_H__

#include "event2/event.h"
#include "business.h"

#include <vector>

struct CLIENT;

class Server final
{
public:
	Server();
	~Server();

public:
	static Server* sharedInstance();

public:
	event_base* getEventBase();

public:
	lw_int32 sendData(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 run(u_short port, LW_PARSE_DATA_CALLFUNC func);

public:
	void listenerCB(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
	void readCB(struct bufferevent *, void *);
	void writeCB(struct bufferevent *, void *);
	void eventCB(struct bufferevent *, short, void *);
	void signalCB(evutil_socket_t, short, void *);

private:
	struct event_base* _base;
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
	std::vector<CLIENT*> vtClients;
};

#endif // !__Server_H__
