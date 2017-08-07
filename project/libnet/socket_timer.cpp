#include "socket_timer.h"

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#include "socket_session.h"

using namespace LW;

struct TIMER_ITEM
{
	SocketTimer* owner;
	struct event* ev;
	int t;
	int tid;
	int state;	// 默认 -1，启动 1，删除 0

public:
	TIMER_ITEM(SocketTimer* owner) : owner(owner), t(0), tid(-1), state(-1)
	{
		ev = new struct event;
	}

	~TIMER_ITEM()
	{
		delete ev;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////

class TimerCore
{
public:
	static void __timer_cb__(evutil_socket_t fd, short event, void *arg)
	{
		TIMER_ITEM *pCurrentTimer = (TIMER_ITEM*)arg;
		pCurrentTimer->owner->__timer_cb(pCurrentTimer, event);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////

SocketTimer::SocketTimer() : _base(nullptr)
{
	
}

SocketTimer::~SocketTimer()
{

}

int SocketTimer::create(struct event_base* base)
{
	this->_base = base;

	return true;
}

void SocketTimer::destroy()
{

}

int SocketTimer::start(int tid, int t, std::function<bool(int id)> func)
{
	if (tid <= 0) return -1;

	this->_on_timer = func;

	TIMER_ITEM* timer = (TIMER_ITEM*)this->_timers[tid];
	if (timer == nullptr)
	{
		timer = new TIMER_ITEM(this);
		_timers[tid] = timer;
	}
	timer->tid = tid;
	timer->t = t;
	timer->state = 1;

	// 设置定时器
	int r = 0;
	r = event_assign(timer->ev, this->_base, -1, 0, TimerCore::__timer_cb__, timer);
	//event_assign(&this->_timer, this->_base, -1, EV_PERSIST, ::time_cb, this);

	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = t;
	tv.tv_usec = 0;

	r = event_add(timer->ev, &tv);
	
	return r;
}

void SocketTimer::kill(int tid)
{
	if (tid <= 0) return;

	TIMER_ITEM* ptimer = (TIMER_ITEM*)this->_timers[tid];
	if (ptimer != nullptr)
	{
		ptimer->state = 0;
	}
}

void SocketTimer::__timer_cb(void* timer, short ev)
{
	TIMER_ITEM* pTimer = (TIMER_ITEM*)timer;
	if (pTimer->tid == 0)
	{
		if (pTimer->state == 0)
		{
			__clean();
		}
	}
	else
	{
		bool r = this->_on_timer(pTimer->tid);
		if (r)
		{
			start(pTimer->tid, pTimer->t, this->_on_timer);
		}
		else
		{
			pTimer->state = 0;
		}

		__clean();
	}
}

void SocketTimer::__clean()
{
	TIMERS::iterator iter = _timers.begin();
	while (iter != _timers.end())
	{
		TIMER_ITEM* pTimer = (TIMER_ITEM*)(iter->second);
		if (pTimer->state == 0)
		{
			event_del(pTimer->ev);
			delete pTimer;
			iter = _timers.erase(iter);
			continue;
		}
		++iter;
	}
}