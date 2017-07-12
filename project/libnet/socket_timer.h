#ifndef __SocketTimer_H__
#define __SocketTimer_H__

#include "event2/event.h"
#include <event2/event_struct.h>
#include "business.h"

#include <string>
#include <unordered_map>

class SocketTimer;
struct TIMER_ITEM;

class SocketTimer final
{
public:
	typedef std::unordered_map<lw_int32, TIMER_ITEM*> TIMERS;

public:
	SocketTimer();
	~SocketTimer();

public:
	int create(struct event_base* base);
	void destory();

public:
	int startTimer(int id, int t, std::function<bool(int id)> func);
	void killTimer(int id);
	void time_cb(void* timer, short ev);

private:
	struct event_base* _base;
	TIMERS _timers;
	std::function<bool(int id)> _on_timer;
};

#endif // !__SocketTimer_H__
