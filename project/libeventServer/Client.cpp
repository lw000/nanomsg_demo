#include "Client.h"
#include "Server.h"

#include <assert.h>

#include "event2/event_compat.h"
#include "event2/event_struct.h"

#define MEM_SIZE    1024

static void on_release(Client* client);
static void do_write(evutil_socket_t sock, short ev, void* arg);
static void do_read(evutil_socket_t sock, short ev, void* arg);


void do_read(evutil_socket_t sock, short ev, void* arg)
{
	Client* client = static_cast<Client*>(arg);
	int result = 0;
	while (1)
	{
		assert(client->write_ev);
		result = ::recv(sock, client->buffer, MEM_SIZE, 0);
		if (result <= 0)
		{
			break;
		}
		printf("recv once, fd: %d, recv size: %d, recv buff: %s\n", sock, result, client->buffer);

		char *p = "222222222222222222";
		::send(sock, p, strlen(p), 0);

		//event_set(client->_write_ev, sock, EV_WRITE, on_write, client->_buffer);
		// 	event_assign(client->write_ev, _evbase, sock, EV_WRITE, do_write, client->buffer);
		// 	event_base_set(_evbase, client->write_ev);
		// 	event_add(client->write_ev, nullptr);
	}

	if (result == 0)
	{
		on_release(client);
		evutil_closesocket(sock);
		return;
	}
	else if (result < 0)
	{
		if (errno == EAGAIN) // XXXX use evutil macro  
			return;

		perror("recv");
		on_release(client);
		evutil_closesocket(sock);
	}
	else
	{
		
	}
}

void do_write(evutil_socket_t sock, short ev, void* arg)
{
	Client* client = static_cast<Client*>(arg);

	::send(sock, client->buffer, strlen(client->buffer), 0);
	// 
	// 	free(buffer);

	event_del(client->write_ev);
}


void on_release(Client* client)
{
	SAFE_DELETE_PTR(client);
}

Client::Client(struct event_base* evbase) : evbase(evbase)
{
	
}

bool Client::create(int newfd)
{
	bool ret = 0;
	do
	{
		this->read_ev = event_new(this->evbase, newfd, EV_READ | EV_PERSIST, do_read, this);
		if (nullptr == this->read_ev)
		{
			ret = false; break;
		}

		this->write_ev = event_new(this->evbase, newfd, EV_WRITE | EV_PERSIST, do_write, this);
		if (nullptr == this->write_ev)
		{
			event_free(this->read_ev);
			ret = false; break;
		}

		this->buffer = (char*)calloc(1, 1024);
		if (nullptr == this->buffer)
		{
			ret = false; break;
		}

		//event_set(_read_ev, _newfd, EV_READ | EV_PERSIST, HNSocketClient::onRead, client);
		// 	event_assign(_read_ev, _evbase, _newfd, EV_READ | EV_PERSIST, onRead, this);
		//event_base_set(_evbase, this->read_ev);
		event_add(this->read_ev, nullptr);

		//event_base_set(_evbase, this->write_ev);
		//event_add(this->write_ev, nullptr);

		ret = true;
	} while (0);
	return ret;
}

Client::~Client()
{
	if (nullptr != this->read_ev)
	{
		event_free(this->read_ev);
	}

	if (nullptr != this->write_ev)
	{
		event_free(this->write_ev);
	}

	if (nullptr != this->buffer)
	{
		free(this->buffer);
		this->buffer = nullptr;
	}
}