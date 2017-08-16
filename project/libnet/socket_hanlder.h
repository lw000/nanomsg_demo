#ifndef __session_handler_h__
#define __session_handler_h__

#include "common_type.h"
#include <functional>

class SocketSession;
class SocketServer;
class SocketClient;

class ISocketSessionHanlder;

typedef std::function<bool(char* buf, lw_int32 bufsize)> SocketCallback;

class ISocketSessionHanlder
{
	friend class SocketSession;

public:
	virtual ~ISocketSessionHanlder() {}

protected:
	virtual int onSocketConnected(SocketSession* session) = 0;
	virtual int onSocketDisConnect(SocketSession* session) = 0;
	virtual int onSocketTimeout(SocketSession* session) = 0;
	virtual int onSocketError(SocketSession* session) = 0;

protected:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) = 0;
};

class ISocketThread
{
	friend class SocketClient;
	friend class SocketServer;

public:
	virtual ~ISocketThread() {}

protected:
	virtual int onStart() = 0;
	virtual int onEnd() = 0;
};

class ISocketClientHandler : public ISocketSessionHanlder, public ISocketThread
{
	friend class SocketClient;
public:
	virtual ~ISocketClientHandler() {}
};

class ISocketServerHandler : public ISocketSessionHanlder, public ISocketThread
{
	friend class SocketServer;

public:
	virtual ~ISocketServerHandler() {}

protected:
	virtual void onListener(SocketSession* session) = 0;
};

#endif // !__session_H__