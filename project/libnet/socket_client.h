#ifndef __SocketClient_H__
#define __SocketClient_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <string>

struct SocketSession;

class SocketClient final
{
public:
	SocketClient();
	~SocketClient();

public:
	bool init();
	void unInit();

public:
	int run(const char* addr, int port);
	bool isConnected();

public:
	int setRecvHook(LW_PARSE_DATA_CALLFUNC func);
	int setTimerHook(std::function<bool(SocketSession* session)> func);

public:
	void time_cb(evutil_socket_t fd, short ev);

private:
	void __run();

public:
	SocketSession* _session;

private:
	struct event_base* _base;
	struct event _timer;

	std::function<bool(SocketSession* session)> _on_timer_func;
};

#endif // !__SocketClient_H__
