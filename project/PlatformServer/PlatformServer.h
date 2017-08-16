#ifndef __PlatformServer_ServerHandler_h__
#define __PlatformServer_ServerHandler_h__

#include "socket_server.h"
#include "SessionManager.h"

class IUser;

class ServerHandler : public ISocketServerHandler
{
public:
	IUser* iuser;

public:
	ServerHandler();
	virtual ~ServerHandler();

protected:
	virtual int onStart() override;
	virtual int onEnd() override;

protected:
	virtual void onListener(SocketSession* session) override;

protected:
	virtual int onSocketConnected(SocketSession* session) override;
	virtual int onSocketDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout(SocketSession* session) override;
	virtual int onSocketError(SocketSession* session) override;

protected:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) override;
};



#endif	// !__PlatformServer_ServerHandler_h__
