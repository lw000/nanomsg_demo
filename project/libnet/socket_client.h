#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <string>

#include "object.h"
#include "socket_session.h"

class SocketSession;
class SocketTimer;
class SocketClient;

struct event_base;

class SocketClient : public Object
{
public:
	SocketClient();
	~SocketClient();

public:
	bool create(ISocketSession* isession);
	void destory();

public:
	int run(const char* addr, int port);
	SocketSession* getSession();

public:
	int startTimer(int id, int t, std::function<bool(int id)> func);
	void killTimer(int id);

private:
	void __run();

private:
	struct event_base* _base;
	SocketSession* _session;
	SocketTimer* _timer;

	ISocketSession* isession;
};

#endif // !__SocketClient_H__
