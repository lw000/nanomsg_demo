#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <string>

#include "socket_session.h"
#include "socket_timer.h"

class ISocketClient;
class SocketClient;
class EventObject;

struct event_base;

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

private:
	ISocketClient* _isession;
};

#endif // !__SocketClient_H__
