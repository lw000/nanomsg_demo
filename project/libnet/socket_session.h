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
	virtual int onSocketTimeout(SocketSession* session) = 0;
	virtual int onSocketError(int error, SocketSession* session) = 0;

public:
	virtual void onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) = 0;
};

enum SESSION_TYPE
{
	Client = 0,
	Server = 1,
};

class SocketSession : public Object
{
	friend class CoreSocket;

public:
	SocketSession(SESSION_TYPE c);
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

private:
	void onRead();
	void onWrite();
	void onEvent(short ev);
	void onParse(lw_int32 cmd, char* buf, lw_int32 bufsize);

private:
	SESSION_TYPE _c;	//session¿‡–Õ

private:
	std::string _host;
	int _port;
	bool _connected;

private:
	struct bufferevent* _bev;
	ISocketSession * _isession;
};


#endif // !__session_H__