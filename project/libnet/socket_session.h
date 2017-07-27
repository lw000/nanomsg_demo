#ifndef __session_H__
#define __session_H__

#include <string>
#include <event2/util.h>

#include "base_type.h"
#include "business.h"
#include "object.h"

class SocketSession;
class ISocketSession;

class ISocketSession
{
public:
	virtual ~ISocketSession() {}

public:
	virtual int onConnected(SocketSession* session) = 0;
	virtual int onDisConnect(SocketSession* session) = 0;
	virtual int onSocketTimeout() = 0;
	virtual int onSocketError(int error, SocketSession* session) = 0;

public:
	virtual void onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) = 0;
};

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
	int create(struct event_base* base, evutil_socket_t fd, short event, ISocketSession* isession);
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

public:
	void read_ev();
	void write_ev();
	void event_ev(short ev);
	void onParse(lw_int32 cmd, char* buf, lw_int32 bufsize);

private:
	TYPE _c;	//session¿‡–Õ

private:
	struct bufferevent* _bev;

private:
	std::string _host;
	int _port;
	bool _connected;

private:
	ISocketSession * isession;
};


#endif // !__session_H__