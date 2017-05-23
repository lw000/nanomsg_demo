// libevent-demo.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

static void do_read(evutil_socket_t fd, short events, void *arg);
static void do_accept(evutil_socket_t listener, short event, void *arg);

//struct bufferevent内建了两个event(read/write)和对应的缓冲区(struct evbuffer *input, *output)，并提供相应的函数用来操作>
//缓冲区(或者直接操作bufferevent)
//接收到数据后，判断是不一样一条消息的结束，结束标志为"over"字符串
void readcb(struct bufferevent *bev, void *ctx)
{
	printf("called readcb!\n");
	struct evbuffer *input, *output;
	char *request_line;
	size_t len;
	input = bufferevent_get_input(bev);//其实就是取出bufferevent中的input
	output = bufferevent_get_output(bev);//其实就是取出bufferevent中的output

	size_t input_len = evbuffer_get_length(input);
	printf("input_len: %d\n", input_len);
	size_t output_len = evbuffer_get_length(output);
	printf("output_len: %d\n", output_len);

	evutil_socket_t fd = bufferevent_getfd(bev);

	while (1)
	{
		int ret = evbuffer_read(input, fd, 1024);
		request_line = evbuffer_readln(input, &len, /*EVBUFFER_EOL_CRLF*/EVBUFFER_EOL_ANY);//从evbuffer前面取出一行，用一个新分配的空字符结束
		//的字符串返回这一行, EVBUFFER_EOL_CRLF表示行尾是一个可选的回车，后随一个换行符
		if (NULL == request_line)
		{
			printf("The first line has not arrived yet.\n");
			::free(request_line);//之所以要进行free是因为 line = mm_malloc(n_to_copy+1))，在这里进行了malloc
			break;
		}
		else
		{
			printf("Get one line date: %s\n", request_line);
			if (strstr(request_line, "over") != NULL)//用于判断是不是一条消息的结束
			{
				char *response = "Response ok! Hello Client!\r\n";
				evbuffer_add(output, response, strlen(response));//Adds data to an event buffer
				printf("服务端接收一条数据完成，回复客户端一条消息: %s\n", response);
				::free(request_line);
				break;
			}
		}
		::free(request_line);
	}

	size_t input_len1 = evbuffer_get_length(input);
	printf("input_len1: %d\n", input_len1);
	size_t output_len1 = evbuffer_get_length(output);
	printf("output_len1: %d\n\n", output_len1);
}

void errorcb(struct bufferevent *bev, short error, void *ctx)
{
	if (error & BEV_EVENT_EOF)
	{
		/* connection has been closed, do any clean up here */
		printf("connection closed\n");
	}
	else if (error & BEV_EVENT_ERROR)
	{
		/* check errno to see what error occurred */
		printf("some other error\n");
	}
	else if (error & BEV_EVENT_TIMEOUT)
	{
		/* must be a timeout event handle, handle it */
		printf("Timed out\n");
	}
	bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
	struct event_base *base = (struct event_base *)arg;
	struct sockaddr_storage ss;
	//struct sockaddr_in sin;
	int slen = sizeof(ss);
	int fd = ::accept(listener, (struct sockaddr*)&ss, &slen);
	if (fd < 0)
	{
		perror("accept");
	}
	else if (fd > FD_SETSIZE)
	{
		evutil_closesocket(fd);
	}
	else
	{
		struct bufferevent *bev;
		int ret = evutil_make_socket_nonblocking(fd);

		//使用bufferevent_socket_new创建一个struct bufferevent *bev，关联该sockfd，托管给event_base
		////BEV_OPT_CLOSE_ON_FREE表示释放bufferevent时关闭底层传输端口。这将关闭底层套接字，释放底层bufferevent等。
		bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

		//设置读写对应的回调函数
		bufferevent_setcb(bev, readcb, nullptr, errorcb, nullptr);
		//      bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);

		//启用读写事件,其实是调用了event_add将相应读写事件加入事件监听队列poll。正如文档所说，如果相应事件不置为true，buf
		//ferevent是不会读写数据的
		bufferevent_enable(bev, EV_READ | EV_WRITE);
	}
}

static void run_server(u_short port = 9876)
{
	struct event_base *base;

	base = event_base_new();
	if (!base)
	{
		return;
	}

	evutil_socket_t listener;
 	listener = ::socket(AF_INET, SOCK_STREAM, 0);

	//允许多次绑定同一个地址。要用在socket和bind之间  
	evutil_make_listen_socket_reuseable(listener);

#ifndef WIN32
	{
		int one = 1;
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	}
#endif

	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(port);
	if (::bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		perror("bind");
		return;
	}

	if (::listen(listener, 16) < 0)
	{
		perror("listen");
		return;
	}

	//跨平台统一接口，将套接字设置为非阻塞状态
	evutil_make_socket_nonblocking(listener);

	struct event *listener_event;
	listener_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, (void*)base);

	event_add(listener_event, nullptr);

	event_base_dispatch(base);

	event_free(listener_event);
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

	//Server server;
	//server.run();

	run_server();

	WSACleanup();

	return 0;
}

#endif // TEST
