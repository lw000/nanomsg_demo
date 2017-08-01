#ifndef __SocketTimer_H__
#define __SocketTimer_H__

#include <unordered_map>

#include "socket_core.h"
#include "object.h"

struct event_base;

class SocketTimer : public Object
{
public:
	typedef std::unordered_map<lw_int32, void*> TIMERS;

public:
	SocketTimer();
	virtual ~SocketTimer();

public:
	int create(struct event_base* base);
	void destory();

public:
	int startTimer(int id, int t, std::function<bool(int id)> func);
	void killTimer(int id);
	void timer_cb(void* timer, short ev);

private:
	struct event_base* _base;
	TIMERS _timers;
	std::function<bool(int id)> _on_timer;
};

#endif // !__SocketTimer_H__
