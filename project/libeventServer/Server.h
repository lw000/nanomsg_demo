#ifndef __HNSocketServer_H__
#define __HNSocketServer_H__

#include "event2/event.h"
#include <hash_map>

class Client;

class Server final
{
public:
	Server();
	~Server();

public:
	static Server* getInstance();

public:
	std::hash_map<int, Client*> SocktClients;

public:
	void run(u_short port = 9876);

public:
	event_base* getEventBase();
	int attach(Client* client);

public:
	void onAccept(evutil_socket_t sock, short event, void* arg);

private:
	struct event_base* _base;
	FILE* f;
};

#endif // !__HNSocketServer_H__
