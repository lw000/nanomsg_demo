#ifndef __session_H__
#define __session_H__

#include "base_type.h"

#include "business.h"

#include <event2/util.h>

#include <string>

class SocketSession
{
public:
	enum TYPE
	{
		Client = 0,
		Server = 1,
	};

public:
	TYPE c;
	struct bufferevent* bev;
	bool connected;
	std::string host;
	int port;
	
	unsigned int uid;

	LW_PARSE_DATA_CALLFUNC _on_recv_func;

public:
	SocketSession(TYPE c);

public:
	evutil_socket_t getSocket();

public:
	int create(struct event_base* base, evutil_socket_t fd, short event, std::string addr = "127.0.0.1", int port = 0);

public:
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);
	int setRecvCall(LW_PARSE_DATA_CALLFUNC func);

public:
	void read_cb();
	void write_cb();
	void event_cb(short ev);
};


#endif // !__session_H__