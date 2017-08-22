#include "Threadable.h"

#ifndef WIN32
#include <pthread.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <stddef.h>
#include <iostream>

class CoreThread
{
public:
	static void* __threadfunc(void *userdata)
	{
		Threadable *self = (Threadable *)userdata;
		
		self->onStart();
		self->run();
		self->onEnd();

		return nullptr;
	}
};

Threadable::Threadable(void) : _threadId(-1)
{
}

Threadable::~Threadable(void)
{
}

int Threadable::yield()
{
#ifndef WIN32
	return sched_yield();
#else
	SwitchToThread();
#endif
	return 0;
}

static int startThread(void* (*threadFun) (void *), void *args)
{
#ifndef WIN32
	int hThread;
	pthread_t recvthread;

	hThread = pthread_create(&recvthread, NULL, threadFun, args);
	if (hThread != 0)
	{
		return -1;
	}
	else
	{
		pthread_detach(recvthread);
	}

	return recvthread;

#else
	DWORD threadHandler;
	DWORD  threadId;
	threadHandler = (int)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFun, args, 0, &threadId);
	if (threadHandler == -1) {
		return -1;
	}

	SetThreadPriority((HANDLE)threadHandler, THREAD_PRIORITY_NORMAL);
	return threadHandler;
#endif
}

void Threadable::start()
{
	_threadId = startThread(CoreThread::__threadfunc, this);
}

void Threadable::join()
{
#ifndef WIN32
	if (_threadId > 0)
	{
		pthread_join(_threadId, NULL);
	}
#endif
}

void Threadable::milliSleep(unsigned long milli)
{
#ifndef WIN32
	usleep(milli * 1000);
#else
	Sleep(milli);
#endif
}