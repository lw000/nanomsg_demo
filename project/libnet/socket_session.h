#ifndef __session_H__
#define __session_H__

#include <string>
#include <event2/util.h>

#include "base_type.h"
#include "business.h"
#include "object.h"

class SocketSession : public Object
{
public:
	enum TYPE
	{
		Client = 0,
		Server = 1,
	};

public:
	SocketSession(TYPE c);
	virtual ~SocketSession();

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

	bool connected();

public:
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize, SocketCallback cb);
	int setRecvCall(LW_PARSE_DATA_CALLFUNC func);

public:
	void read_ev();
	void write_ev();
	void event_ev(short ev);

private:
	TYPE _c;	//session����

private:
	struct bufferevent* _bev;

private:
	std::string _host;
	int _port;
	bool _connected;

private:
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
};


#endif // !__session_H__