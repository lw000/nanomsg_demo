#ifndef __SimpleMessageQueue_h__
#define __SimpleMessageQueue_h__

#include <queue>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <time.h>

class Msgdata {
	long   mtype;
	unsigned char*  mtext;
	unsigned long   mtext_l;
	time_t stime;
	time_t rtime;
	time_t ctime;

public:
	Msgdata();
	Msgdata(int mtype, unsigned char* mtext, unsigned long mtext_l);
	~Msgdata();

public:
	Msgdata(const Msgdata& other);
	Msgdata(Msgdata&& other);
	Msgdata& operator=(const Msgdata& other);
	Msgdata& operator=(Msgdata&& other);

public:
	long getmtype() const;
	unsigned char* getmtext() const;
	long getmtextl() const;
};

class MsgQueueItem
{
public:
	MsgQueueItem() {
		_total = 0;
	}

	~MsgQueueItem() {

	}

private:
	std::queue<Msgdata> _msg;
	std::atomic<long long> _total;
};

class SimpleMessageBroker
{
public:
	SimpleMessageBroker();
	~SimpleMessageBroker();

private:

};

class SimpleMessageChannel
{
public:
	SimpleMessageChannel();
	~SimpleMessageChannel();

private:

};

class SimpleMessageQueue final
{
public:
	SimpleMessageQueue();
	~SimpleMessageQueue();

public:
	int createChannel(const char* channel = NULL);
	void releaseChannel(const char* channel);

public:
	void push(const Msgdata& msg, const char* channel = NULL);
	Msgdata pop(const char* channel = NULL);

	bool empty();
	long long size();

private:
	std::mutex _m;
	std::unordered_map<unsigned long, std::queue<Msgdata>> _msg_queue;
	std::atomic<long long> _msg_total;
};

#endif	// !__SimpleMessageQueue_h__
