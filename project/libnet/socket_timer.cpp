#include "socket_timer.h"

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

#include <thread>

#include "socket_processor.h"

struct TIMER_ITEM
{
	ITimer* owner;
	unsigned int begin;
	unsigned int end;

	int tms;		// 时间（毫秒）

//#ifdef _WIN32
	int real_tid;	// 内核定时器ID
//#else
	struct event* ev;	// 内核定时器事件
//#endif	// !_WIN32

	int tid;		// 用户定时器ID

	TIMER_ITEM(ITimer* owner) : owner(owner), tms(-1), tid(-1), real_tid(-1), begin(0), end(0)
	{
//#ifndef _WIN32
		ev = new struct event;
//#endif	// !_WIN32
	}

	~TIMER_ITEM()
	{
//#ifndef _WIN32
		delete ev;
//#endif	// !_WIN32
	}

	bool operator == (const TIMER_ITEM& item)
	{
		return (this->tid == item.tid) && (this->tms == item.tms) ? true : false;
	}
};

struct timer_item_hash_func  // hash函数  
{
	size_t operator()(const TIMER_ITEM& item) const
	{
		return item.tid;
	}
};

struct timer_item_cmp_fun // 比较函数 ==  
{
	bool operator()(const TIMER_ITEM& item1, const TIMER_ITEM& item2) const
	{
		return (item1.tid == item2.tid) && (item1.tms == item2.tms) ? true : false;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////

class TimerCore
{
public:
//#ifdef _WIN32

	static void CALLBACK __timer_cb_win32__(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
	{
		TIMER_ITEM *pCurrentTimer = (TIMER_ITEM*)dwUser;
		if (pCurrentTimer != nullptr)
		{
			pCurrentTimer->owner->_timer_cb(pCurrentTimer);
		}
	}

//#else

	static void  __timer_cb__(evutil_socket_t fd, short event, void *arg)
	{
		TIMER_ITEM *pCurrentTimer = (TIMER_ITEM*)arg;
		if (pCurrentTimer != nullptr)
		{
			pCurrentTimer->owner->_timer_cb(pCurrentTimer);
		}
	}

//#endif
};

//////////////////////////////////////////////////////////////////////////////////////////
//#ifdef _WIN32

class TimerWin32 : public Object, public ITimer
{
public:
	TimerWin32();
	virtual ~TimerWin32();

public:
	virtual int create(SocketProcessor* base = nullptr) override;
	virtual void destroy() override;

public:
	virtual int start(int tid, int tms, TIMER_CALLBACK func) override;
	virtual int start_once(int tid, int tms, TIMER_CALLBACK func) override;
	virtual void kill(int tid) override;

public:
	std::string debug() override;

protected:
	virtual void _timer_cb(TIMER_ITEM* timer) override;

private:
	int __start(int tid, int tms, TIMER_CALLBACK func, unsigned int fuEvent);

private:
	TIMERS _timers;
	TIMER_CALLBACK _on_timer;
};

TimerWin32::TimerWin32()
{
	TIMECAPS caps;
	UINT r = ::timeGetDevCaps(&caps, sizeof(caps));
	UINT r1 = ::timeBeginPeriod(10);
}

TimerWin32::~TimerWin32()
{
	TIMERS::iterator iter = _timers.begin();
	while (iter != _timers.end())
	{
		TIMER_ITEM* ptimer = iter->second;

		UINT err = 0;
		err = ::timeKillEvent(ptimer->tid);

		delete ptimer;

		iter = _timers.erase(iter);
	}

	UINT r = ::timeEndPeriod(10);
}

int TimerWin32::create(SocketProcessor* base)
{
	return true;
}

void TimerWin32::destroy()
{
}

std::string TimerWin32::debug()
{
	return std::string("TimerWin32");
}

int TimerWin32::start(int tid, int tms, TIMER_CALLBACK func)
{
	int r = this->__start(tid, tms, func, TIME_PERIODIC);
	return r;
}

int TimerWin32::start_once(int tid, int tms, TIMER_CALLBACK func)
{
	int r = this->__start(tid, tms, func, TIME_ONESHOT);
	return r;
}

void TimerWin32::kill(int tid)
{
	if (tid < 0) return;

	TIMER_ITEM* ptimer = this->_timers[tid];
	if (ptimer == nullptr) return;

	UINT err = 0;
	err = ::timeKillEvent(ptimer->real_tid);
	if (err == TIMERR_NOERROR)
	{
		TIMERS::iterator iter = _timers.begin();
		while (iter != _timers.end())
		{
			TIMER_ITEM* ptimer = iter->second;
			if (ptimer->tid == tid)
			{
				delete ptimer;
				ptimer = nullptr;

				iter = _timers.erase(iter);
				break;
			}
			else
			{
				++iter;
			}
		}
	}
}

int TimerWin32::__start(int tid, int tms, TIMER_CALLBACK func, unsigned int fuEvent)
{
	if (tid < 0) return -1;

	this->_on_timer = func;

	TIMER_ITEM* ptimer = this->_timers[tid];
	if (ptimer != nullptr)
	{
		UINT err = 0;
		err = ::timeKillEvent(ptimer->real_tid);
	}
	
	ptimer = new TIMER_ITEM(this);

	UINT real_tid = 0;
	real_tid = ::timeSetEvent(tms, 10, TimerCore::__timer_cb_win32__, (DWORD_PTR)ptimer, fuEvent);
	if (tid != 0)
	{
		ptimer->tms = tms;
		ptimer->tid = tid;
		ptimer->begin = ::timeGetTime();
		ptimer->end = ptimer->begin;
		ptimer->real_tid = real_tid;
		
		_timers[tid] = ptimer;
	}
	else
	{
		UINT err = 0;
		err = ::timeKillEvent(ptimer->real_tid);
		delete ptimer;
		ptimer = nullptr;
	}

	return tid;
}

void TimerWin32::_timer_cb(TIMER_ITEM* timer)
{
	timer->end = ::timeGetTime();
	unsigned int new_begin = timer->begin;
	timer->begin = timer->end;
	bool r = this->_on_timer(timer->tid, timer->end - new_begin);
	if (!r)
	{
		this->kill(timer->tid);
	}
}

//#else

class TimerLinux : public Object, public ITimer
{
public:
	TimerLinux();
	virtual ~TimerLinux();

public:
	virtual int create(SocketProcessor* processor = nullptr) override;
	virtual void destroy() override;

public:
	virtual int start(int tid, int tms, TIMER_CALLBACK func) override;
	virtual int start_once(int tid, int tms, TIMER_CALLBACK func) override;
	virtual void kill(int tid) override;

public:
	std::string debug() override;

protected:
	virtual void _timer_cb(TIMER_ITEM* timer) override;

private:
	SocketProcessor* _processor;
	TIMERS _timers;
	TIMER_CALLBACK _on_timer;
};

//////////////////////////////////////////////////////////////////////////////////////////

TimerLinux::TimerLinux() : _processor(nullptr)
{

}

TimerLinux::~TimerLinux()
{

}

int TimerLinux::create(SocketProcessor* processor)
{
	this->_processor = processor;

	return true;
}

void TimerLinux::destroy()
{
	TIMERS::iterator iter = _timers.begin();
	while (iter != _timers.end())
	{
		TIMER_ITEM* pTimer = iter->second;

		event_del(pTimer->ev);

		delete pTimer;

		iter = _timers.erase(iter);
	}
}

std::string TimerLinux::debug()
{
	return std::string("TimerLinux");
}

int TimerLinux::start(int tid, int tms, TIMER_CALLBACK func)
{
	if (tid < 0) return -1;

	this->_on_timer = func;

	TIMER_ITEM* ptimer = this->_timers[tid];
	if (ptimer == nullptr)
	{
		ptimer = new TIMER_ITEM(this);
		_timers[tid] = ptimer;
	}
	else
	{
		int r = 0;
		r = event_del(ptimer->ev);
	}

	ptimer->tid = tid;
	ptimer->tms = tms;

	// 设置定时器
	int r = 0;
	/* 定时器只执行一次，每次需要添加*/
	//r = event_assign(ptimer->ev, this->_base, -1, 0, TimerCore::__timer_cb__, ptimer);

	/* 定时器只执行无限次执行*/
	r = event_assign(ptimer->ev, this->_processor->getBase(), -1, EV_PERSIST, TimerCore::__timer_cb__, ptimer);

	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = tms / 1000;
	tv.tv_usec = (tms % 1000) * 1000;

	r = event_add(ptimer->ev, &tv);

	return r;
}

int TimerLinux::start_once(int tid, int tms, TIMER_CALLBACK func)
{

	return 0;
}

void TimerLinux::kill(int tid)
{
	if (tid < 0) return;

	TIMER_ITEM* ptimer = this->_timers[tid];
	if (ptimer != nullptr)
	{
		TIMERS::iterator iter = _timers.begin();
		while (iter != _timers.end())
		{
			TIMER_ITEM* timer = iter->second;
			if (timer->tid == tid)
			{
				int r = 0;
				r = event_del(ptimer->ev);

				delete timer;

				iter = _timers.erase(iter);

				break;
			}
			++iter;
		}
	}
}

void TimerLinux::_timer_cb(TIMER_ITEM* timer)
{
	TIMER_ITEM* ptimer = timer;
	bool r = this->_on_timer(ptimer->tid, 0);
	if (!r)
	{
		this->kill(ptimer->tid);
	}
}

//#endif

Timer::Timer() : _timer(nullptr)
{
//#ifdef _WIN32
	//_timer = new TimerWin32;
//#else
	_timer = new TimerLinux;
//#endif
}

Timer::~Timer()
{
	
}

int Timer::create(SocketProcessor* processor)
{
	return _timer->create(processor);
}

void Timer::destroy()
{
	_timer->destroy();
}

int Timer::start(int tid, int tms, TIMER_CALLBACK func)
{
	return _timer->start(tid, tms, func);
}

int Timer::start_once(int tid, int tms, TIMER_CALLBACK func)
{
	return _timer->start_once(tid, tms, func);
}

void Timer::kill(int tid)
{
	_timer->kill(tid);
}

std::string Timer::debug()
{
	return std::string("Timer");
}