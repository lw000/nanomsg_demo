#ifndef __SocketClient_H__
#define __SocketClient_H__

#include <string>

#include "business.h"
#include "object.h"

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
	bool create();
	void destory();

public:
	int run(const char* addr, int port);
	SocketSession* getSession();

public:
	int setRecvHook(LW_PARSE_DATA_CALLFUNC func, void* userdata);
	
public:
	int startTimer(int id, int t, std::function<bool(int id)> func);
	void killTimer(int id);

private:
	void __run();

private:
	struct event_base* _base;
	SocketSession* _session;
	SocketTimer* _timer;
};

#endif // !__SocketClient_H__
