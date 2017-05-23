// libeventClient.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>

#include "event2/event.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>

void on_read_cb(struct bufferevent *bev, void *ctx)
{
	char sss[1024];
	bufferevent_read(bev, sss, 1024);
}

void on_wrire_cb(struct bufferevent *bev, void *ctx)
{

}

void on_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	const char *data = (const char *)ctx;
	printf("Got an event on socket %s%s%s%s [%s]",
		(what&EV_TIMEOUT) ? " timeout" : "",
		(what&EV_READ) ? " read" : "",
		(what&EV_WRITE) ? " write" : "",
		(what&EV_SIGNAL) ? " signal" : "",
		data);
}

#if FALSE
int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	WSADATA WSAData;
	int ret;
	if (ret = WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cout << "Can not initilize winsock.dll" << std::endl;
		std::cout << "Error Code:" << WSAGetLastError() << std::endl;
		return 1;
	}
	else
	{
		std::cout << "winsock.dll load" << std::endl;
	}
#endif

	// build socket
	int port = 9876;
	struct sockaddr_in my_address;
	memset(&my_address, 0, sizeof(my_address));
	my_address.sin_family = AF_INET;
	my_address.sin_addr.s_addr = htonl(0x7f000001); // 127.0.0.1
	my_address.sin_port = htons(port);

	// build event base
	struct event_base* base = event_base_new();

	// set TCP_NODELAY to let data arrive at the server side quickly
	evutil_socket_t fd;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	struct bufferevent* conn = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	int enable = 1;
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&enable, sizeof(enable)) < 0)
		printf("ERROR: TCP_NODELAY SETTING ERROR!\n");

	//bufferevent_setcb(conn, NULL, NULL, NULL, NULL); // For client, we don't need callback function
	bufferevent_enable(conn, EV_WRITE);
	if (bufferevent_socket_connect(conn, (struct sockaddr*)&my_address, sizeof(my_address)) == 0)
		printf("connect success\n");

	//bufferevent_setcb(conn, on_read_cb, on_wrire_cb, on_event_cb, "a");

	//while (1)
	{
		char *p = "11111111111111111111111111111111111111111111111111111111111111";
		// start to send data
		bufferevent_write(conn, p, strlen(p));
		// check the output evbuffer
		struct evbuffer* output = bufferevent_get_output(conn);
		// 		char sss[1024];
		// 		bufferevent_read(conn, sss, 1024);
		int len = 0;
		len = evbuffer_get_length(output);
		printf("output buffer has %d bytes left\n", len);
		event_base_dispatch(base);

		//::Sleep(1000);
	}

	bufferevent_free(conn);

	event_base_free(base);

	printf("Client program is over\n");

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

#endif 

