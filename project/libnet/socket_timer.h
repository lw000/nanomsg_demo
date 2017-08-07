#ifndef __SocketTimer_H__
#define __SocketTimer_H__

#include <unordered_map>

#include "socket_core.h"
#include "object.h"

struct event_base;

class SocketTimer : public Object
{
	friend class TimerCore;

public:
	typedef std::unordered_map<lw_int32, void*> TIMERS;

public:
	SocketTimer();
	virtual ~SocketTimer();

public:
	int create(struct event_base* base);
	void destroy();

public:
	int start(int tid, int t, std::function<bool(int id)> func);
	void kill(int tid);

private:
	void __clean();
	void __timer_cb(void* timer, short ev);

private:
	struct event_base* _base;
	TIMERS _timers;
	std::function<bool(int id)> _on_timer;
};

#endif // !__SocketTimer_H__
