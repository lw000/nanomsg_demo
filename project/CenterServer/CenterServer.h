#ifndef __CenterServer_ServerHandler_h__
#define __CenterServer_ServerHandler_h__

#include "socket_server.h"
#include "SessionManager.h"

class ServerHandler : public ISocketServerHandler
{
public:
	ManagerT<SocketSession> Sessions;

public:
	ServerHandler();
	virtual ~ServerHandler();

public:
	virtual void onListener(SocketSession* session) override;

protected:
	virtual int onSocketConnected(SocketSession* session) override;
	virtual int onSocketDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout(SocketSession* session) override;
	virtual int onSocketError(SocketSession* session) override;

protected:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) override;

private:
	void sendHeartbeat(SocketSession* session);
};

#endif	// !__CenterServer_ServerHandler_h__
