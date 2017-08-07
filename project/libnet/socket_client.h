#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <string>

#include "event_object.h"
#include "socket_session.h"
#include "socket_timer.h"

class SocketClient;

struct event_base;

class SocketClient : public EventObject
{
public:
	SocketClient();
	virtual ~SocketClient();

public:
	bool create(ISocketSessionHanlder* isession);
	void destroy();

public:
	int run(const char* addr, int port);

public:
	SocketSession* getSession();
	SocketTimer* getTimer();

private:
	void __run();

private:
	SocketSession* _session;
	SocketTimer* _timer;

private:
	ISocketSessionHanlder* isession;
};

#endif // !__SocketClient_H__
