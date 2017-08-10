#ifndef __session_H__
#define __session_H__

#include <string>
#include <event2/util.h>

#include "base_type.h"
#include "object.h"
#include "socket_core.h"
#include "socket_hanlder.h"

#include <unordered_map>

class SocketSession;
class SocketProcessor;

typedef std::function<bool(char* buf, lw_int32 bufsize)> SocketCallback;

enum SESSION_TYPE
{
	NONE = 0,
	Client = 1,
	Server = 2,
};

class SocketSession : public Object, public ISocketSessionHanlder
{
	friend class CoreSocket;

public:
	SocketSession(ISocketSessionHanlder* isession);
	virtual ~SocketSession();

public:
	evutil_socket_t getSocket();

public:
	int create(SESSION_TYPE c, SocketProcessor* base, evutil_socket_t fd, short event);
	int create(SESSION_TYPE c, SocketProcessor* base, evutil_socket_t fd, short event, ISocketSessionHanlder* isession);
	void destroy();

public:
	void setHost(const std::string& host);
	std::string getHost();

	void setPort(int port);
	int getPort();

public:
	bool connected();

public:
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize, lw_int32 recvcmd, SocketCallback cb);

private:
	void onRead();
	void onWrite();
	void onEvent(short ev);
	void reset();

protected:
	virtual int onSocketConnected(SocketSession* session) override;
	virtual int onSocketDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout(SocketSession* session) override;
	virtual int onSocketError(SocketSession* session) override;

protected:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, char* buf, lw_int32 bufsize) override;

private:
	std::unordered_map<lw_int32, SocketCallback> _cmd_event_map;

private:
	SESSION_TYPE _c;	//session¿‡–Õ

private:
	std::string _host;
	int _port;
	bool _connected;

private:
	struct bufferevent* _bev;
	ISocketSessionHanlder * _isession;
};


#endif // !__session_H__