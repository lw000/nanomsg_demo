#ifndef __SocketTimer_H__
#define __SocketTimer_H__

#include "object.h"

#include <unordered_map>
#include <functional>

class SocketProcessor;

struct TIMER_ITEM;

typedef std::function<bool(int tid, unsigned int tms)> TIMER_CALLBACK;

class ITimer
{
	friend class TimerCore;

public:
	typedef std::unordered_map<lw_int32, TIMER_ITEM*> TIMERS;
	typedef std::unordered_map<lw_int32, TIMER_ITEM*>::iterator iterator;
	typedef std::unordered_map<lw_int32, TIMER_ITEM*>::const_iterator const_iterator;

public:
	virtual ~ITimer() {}

public:
	virtual int create(SocketProcessor* processor = nullptr) = 0;
	virtual void destroy() = 0;

public:
	virtual int start(int tid, int tms, TIMER_CALLBACK func) = 0;
	virtual int start_once(int tid, int tms, TIMER_CALLBACK func) = 0;
	virtual void kill(int tid) = 0;

protected:
	virtual void _timer_cb(TIMER_ITEM* timer) = 0;
};

class Timer : public Object
{
public:
	Timer();
	virtual ~Timer();

public:
	int create(SocketProcessor* processor = nullptr);
	void destroy();

public:
	int start(int tid, int tms, TIMER_CALLBACK func);
	int start_once(int tid, int tms, TIMER_CALLBACK func);
	void kill(int tid);

private:
	ITimer* _timer;
};

#endif // !__SocketTimer_H__
