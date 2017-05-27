#ifndef __HNSocketClient_H__
#define __HNSocketClient_H__

#include "event2/event.h"

class Server;

class Client
{
public:
	int newfd;

public:
	struct event_base* evbase;
	struct event* read_ev;
	struct event* write_ev;
	char* buffer;

public:
	Client(struct event_base* evbase);
	~Client();

public:
	bool create(int newfd);
};

#endif // !__HNSocketClient_H__
