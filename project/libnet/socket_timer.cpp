﻿#include "socket_timer.h"

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
	int id;
	int state;	// 默认 -1，启动 1，删除 0

public:
	TIMER_ITEM(SocketTimer* owner) : owner(owner), t(0), id(-1), state(-1)
	{
		ev = new struct event;
	}

	~TIMER_ITEM()
	{
		delete ev;
	}
};


static void __timer_cb(evutil_socket_t fd, short event, void *arg)
{
	if (event & BEV_EVENT_READING)
	{
	}
	else if (event & BEV_EVENT_WRITING)
	{
	}
	else if (event & BEV_EVENT_EOF)
	{
	}
	else if (event & BEV_EVENT_TIMEOUT)
	{
	}
	else if (event & BEV_EVENT_ERROR)
	{

	}
	else if (event & BEV_EVENT_CONNECTED)
	{

	}

	TIMER_ITEM *pCurrentTimer = (TIMER_ITEM*)arg;
	pCurrentTimer->owner->timer_cb(pCurrentTimer, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SocketTimer::SocketTimer() : _base(NULL)
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

void SocketTimer::destory()
{

}

int SocketTimer::startTimer(int id, int t, std::function<bool(int id)> func)
{
	if (id <= 0) return -1;

	this->_on_timer = func;

	int r = 0;

	TIMER_ITEM* timer = (TIMER_ITEM*)this->_timers[id];
	if (timer == NULL)
	{
		TIMER_ITEM* new_timer = new TIMER_ITEM(this);
		new_timer->id = id;
		new_timer->t = t;
		new_timer->state = 1;

		// 设置定时器
		r = event_assign(new_timer->ev, this->_base, -1, 0, ::__timer_cb, new_timer);
		//event_assign(&this->_timer, this->_base, -1, EV_PERSIST, ::time_cb, this);

		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = t;
		tv.tv_usec = 0;

		r = event_add(new_timer->ev, &tv);

		_timers[id] = new_timer;
	}
	else
	{
		timer->id = id;
		timer->t = t;
		timer->state = 1;

		// 设置定时器
		r = event_assign(timer->ev, this->_base, -1, 0, ::__timer_cb, timer);
		//event_assign(&this->_timer, this->_base, -1, EV_PERSIST, ::time_cb, this);

		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = t;
		tv.tv_usec = 0;
		r = event_add(timer->ev, &tv);
	}	

	return r;
}

void SocketTimer::killTimer(int id)
{
	if (id <= 0) return;

	TIMER_ITEM* ptimer = (TIMER_ITEM*)_timers[id];
	if (ptimer != NULL)
	{
		ptimer->state = 0;
	}
}

void SocketTimer::timer_cb(void* timer, short ev)
{
	TIMER_ITEM* pTimer = (TIMER_ITEM*)timer;
	bool r = this->_on_timer(pTimer->id);
	if (r)
	{
		startTimer(pTimer->id, pTimer->t, this->_on_timer);
	}
	else
	{
		pTimer->state = 0;
	}

	TIMERS::iterator iter = _timers.begin();
	while (iter != _timers.end())
	{
		TIMER_ITEM* pTimer = (TIMER_ITEM*)(iter->second);
		if (pTimer->t == 0)
		{
			delete pTimer;
			iter = _timers.erase(iter);
			continue;
		}
		++iter;
	}
}