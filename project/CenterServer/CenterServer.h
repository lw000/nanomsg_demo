#ifndef __CenterServer_ServerHandler_h__
#define __CenterServer_ServerHandler_h__

#include "socket_server.h"

class ServerHandler : public ISocketServer
{
public:
	typedef std::list<SocketSession*> SESSIONS;

public:
	SESSIONS sessions;

public:
	ServerHandler();
	virtual ~ServerHandler();

public:
	virtual void onJoin(SocketSession* session) override;

public:
	virtual int onConnected(SocketSession* session) override;
	virtual int onDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout(SocketSession* session) override;
	virtual int onSocketError(int error, SocketSession* session) override;

public:
	virtual void onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) override;
};

#endif	// !__CenterServer_ServerHandler_h__
