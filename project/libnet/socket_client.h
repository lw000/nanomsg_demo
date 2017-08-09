#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <string>
#include "object.h"
#include "socket_hanlder.h"

class ISocketClient;
class SocketClient;
class EventObject;

class SocketClient : public Object, public ISocketSessionHanlder
{
public:
	SocketClient(EventObject* evObject, ISocketClient* isession);
	virtual ~SocketClient();

public:
	bool create();
	void destroy();

public:
	int run(const std::string& addr, int port);

public:
	SocketSession* getSession();

public:
	virtual std::string debug() override;

protected:
	virtual int onSocketConnected(SocketSession* session) override;
	virtual int onSocketDisConnect(SocketSession* session) override;
	virtual int onSocketTimeout(SocketSession* session) override;
	virtual int onSocketError(SocketSession* session) override;

protected:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, char* buf, lw_int32 bufsize) override;

private:
	void __run();

private:
	EventObject* _evObject;
	SocketSession* _session;
	ISocketClient* _isession;
};

#endif // !__SocketClient_H__
