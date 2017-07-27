#ifndef __CenterServer_h__
#define __CenterServer_h__

#include "socket_server.h"

class CenterServerHandler : public ISocketServer
{
public:
	typedef std::list<SocketSession*> SESSIONS;

public:
	SESSIONS sessions;

public:
	CenterServerHandler();
	virtual ~CenterServerHandler();

public:
	virtual void onJoin(SocketSession* session) override;

public:
	virtual int onConnected(SocketSession* session) override;
	virtual int onDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout() override;
	virtual int onSocketError(int error, SocketSession* session) override;

public:
	virtual void onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) override;
};



#endif	// !__CenterServer_h__
