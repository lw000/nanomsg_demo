#ifndef __session_handler_h__
#define __session_handler_h__

#include "base_type.h"
#include <functional>

class SocketSession;
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


class ISocketClient
{
	friend class SocketClient;

public:
	virtual ~ISocketClient() {}

protected:
	virtual int onSocketConnected() = 0;
	virtual int onSocketDisConnect() = 0;
	virtual int onSocketTimeout() = 0;
	virtual int onSocketError() = 0;

protected:
	virtual void onSocketParse(lw_int32 cmd, char* buf, lw_int32 bufsize) = 0;
};

class ISocketServerHandler : public ISocketSessionHanlder
{
public:
	virtual ~ISocketServerHandler() {}

public:
	virtual void onListener(SocketSession* session) = 0;
};

#endif // !__session_H__