#ifndef __session_H__
#define __session_H__

#include <string>
#include <unordered_map>

#include <event2/util.h>

#include "common_type.h"
#include "object.h"

#include "socket_hanlder.h"

class SocketSession;
class SocketProcessor;

typedef std::function<bool(char* buf, lw_int32 bufsize)> SocketCallback;

enum SESSION_TYPE
{
	NONE = 0,
	Client = 1,
	Server = 2,
};

class SocketSession : public Object
{
	friend class CoreSocket;

public:
	SocketSession(AbstractSocketSessionHanlder* handler);
	virtual ~SocketSession();

public:
	int create(SESSION_TYPE c, SocketProcessor* processor, evutil_socket_t fd, short ev);
	void destroy();

public:
	void setConnTimeout(int s);
	void setRecvTimeout(int s);
	void setSendTimeout(int s);

public:
	void setHost(const std::string& host);
	std::string getHost() const;

	void setPort(int port);
	int getPort() const;

public:
	bool connected();

public:
	virtual std::string debug() override;

public:
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize, lw_int32 recvcmd, SocketCallback cb);

private:
	void __onRead();
	void __onWrite();
	void __onEvent(short ev);
	void __onParse(lw_int32 cmd, char* buf, lw_int32 bufsize);

private:
	void __reset();

private:
	std::unordered_map<lw_int32, SocketCallback> _cmd_event_map;

private:
	SESSION_TYPE _c;	//session¿‡–Õ

private:
	std::string _host;
	int _port;
	bool _connected;

private:
	SocketProcessor* _processor;
	struct bufferevent* _bev;
	AbstractSocketSessionHanlder * _handler;
};


#endif // !__session_H__