#ifndef __SocketClient_H__
#define __SocketClient_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <string>

class SocketClient final
{
public:
	SocketClient();
	~SocketClient();

public:
	bool init();
	void unInit();

public:
	int start(const char* addr, int port);
	bool isConnected() { return connected; }
	lw_int32 sendData(lw_int32 cmd, void* object, lw_int32 objectSize);

public:
	int setRecvHook(LW_PARSE_DATA_CALLFUNC func);
	int setTimerHook(std::function<bool(struct bufferevent* bev)> func);

public:
	void read_cb(struct bufferevent* bev);
	void event_cb(struct bufferevent *bev, short ev);
	void time_cb(evutil_socket_t fd, short ev);

private:
	void __run();

private:
	struct event_base* _base;
	struct event _timer;
	struct bufferevent* _bev;
	int _port;
	bool connected;
	std::string _addr;
	LW_PARSE_DATA_CALLFUNC _on_recv_func;
	std::function<bool(struct bufferevent* bev)> _on_timer_func;
};

#endif // !__SocketClient_H__
