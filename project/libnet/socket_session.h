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

private:
	TYPE c;
	struct bufferevent* _bev;
	bool _connected;
	std::string _host;
	int _port;

private:
	unsigned int uid;
	std::string ext;

private:
	LW_PARSE_DATA_CALLFUNC _on_recv_func;

public:
	SocketSession(TYPE c);
	~SocketSession();

public:
	evutil_socket_t getSocket();

public:
	int create(struct event_base* base, evutil_socket_t fd, short event);
	void destory();

public:
	void setHost(std::string host);
	std::string getHost();

	void setPort(int port);
	int getPort();

	bool getConnected() { return _connected; }

public:
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);
	int setRecvCall(LW_PARSE_DATA_CALLFUNC func);

public:
	void read_ev();
	void write_ev();
	void event_ev(short ev);
};


#endif // !__session_H__