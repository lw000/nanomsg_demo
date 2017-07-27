#ifndef __PlatformServer_h__
#define __PlatformServer_h__

#include "socket_server.h"

class PlatformServerHandler : public ISocketServer
{
public:
	typedef std::list<SocketSession*> SESSIONS;

public:
	SESSIONS sessions;

public:
	PlatformServerHandler();
	virtual ~PlatformServerHandler();

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



#endif	// !__PlatformServer_h__
