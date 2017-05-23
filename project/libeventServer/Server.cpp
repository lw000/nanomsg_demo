#include "Server.h"
#include "Client.h"

#include <assert.h>

#include "event2/event_compat.h"
#include "event2/event_struct.h"

#include "common_marco.h"

#define BACKLOG     5

static Server * gp_server = nullptr;

static void do_accept(evutil_socket_t listener, short event, void* arg)
{
	Server::getInstance()->onAccept(listener, event, arg);
}

Server* Server::getInstance()
{
	return gp_server;
}

Server::Server()
{
	gp_server = this;
	
	_base = event_base_new();

// 	fopen("ev.log", "w");
}

Server::~Server()
{
	event_base_free(_base);
//	::fclose(f);
}

void Server::run(u_short port)
{
	evutil_socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);

	evutil_make_socket_nonblocking(sock);

#ifndef WIN32
	int opt = 1;
	::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));
#endif

	struct sockaddr_in addr;
	::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (::bind(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr)) > 0)
	{
		perror("bind");
		return;
	}

	if (::listen(sock, BACKLOG) < 0)
	{
		perror("listen");
		return;
	}

	//struct event listen_ev;
	////event_set(&listen_ev, sock, EV_READ | EV_PERSIST, do_accept, nullptr);
	//event_assign(&listen_ev, _evbase, sock, EV_READ | EV_PERSIST, do_accept, nullptr);
	//event_base_set(_evbase, &listen_ev);
	//event_add(&listen_ev, nullptr);

	struct event* listen_ev;
	listen_ev = event_new(_base, sock, EV_READ | EV_PERSIST, do_accept, nullptr);
	event_base_set(_base, listen_ev);
	event_add(listen_ev, nullptr);

// 	event_base_dump_events(_evbase, f);

	event_base_dispatch(_base);

	event_free(listen_ev);
}

event_base* Server::getEventBase()
{
	return _base;
}

int Server::attach(Client* client)
{
	SocktClients[client->newfd] = client;
	return 0;
}

void Server::onAccept(evutil_socket_t listener, short event, void* arg)
{
	struct sockaddr_in addr;
	int addr_size = sizeof(struct sockaddr_in);
	int newfd = ::accept(listener, (struct sockaddr*)&addr, &addr_size);
	do
	{
		if (newfd < 0)
		{
			perror("accept(): can not accept client connection");
			break;
		}
		
		int ret = evutil_make_socket_nonblocking(newfd);

		Client* client = new Client(_base);
		if (!client->create(newfd))
		{
			SAFE_DELETE(client); 
			break;	
		}
		attach(client);

	} while (0);
}