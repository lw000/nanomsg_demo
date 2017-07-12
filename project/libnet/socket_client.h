#ifndef __SocketClient_H__
#define __SocketClient_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <string>
#include <unordered_map>

class SocketSession;
class SocketClient;
class SocketTimer;

typedef std::function<bool(int id, SocketSession* session)> CLIENT_TIMERCALL;

class SocketClient final
{
public:
	SocketClient();
	~SocketClient();

public:
	bool create();
	void destory();

public:
	int run(const char* addr, int port);
	SocketSession* getSession();

public:
	int setRecvHook(LW_PARSE_DATA_CALLFUNC func, void* userdata);
	
public:
	int startTimer(int id, int t, CLIENT_TIMERCALL func);
	void killTimer(int id);

private:
	void __run();

private:
	struct event_base* _base;
	SocketSession* _session;
	SocketTimer* _timer;
	CLIENT_TIMERCALL _on_timer_func;
};

#endif // !__SocketClient_H__
