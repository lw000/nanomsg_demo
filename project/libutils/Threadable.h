#ifndef __Threadable_h__
#define __Threadable_h__

class Threadable
{
	friend class CoreThread;

public:
	static void milliSleep(unsigned long milli);

public:
	Threadable();
	virtual ~Threadable();

public:
	void start();
	void join();
	int yield();

protected:
	virtual int onStart() = 0;
	virtual int run() = 0;
	virtual int onEnd() = 0;

private:
	int _threadId;
};


#endif	// !__Threadable_h__

