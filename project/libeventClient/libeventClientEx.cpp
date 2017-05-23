// libeventClient.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>

#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/util.h>

#define BUF_SIZE	1024

static int connect_server(const char* server_ip, int port);

static void cmd_msg_cb(int fd, short events, void* arg);
static void read_cb(int fd, short events, void *arg);
static void signal_cb(evutil_socket_t, short, void *);

static void signal_cb(evutil_socket_t fd, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 1, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

static void cmd_msg_cb(int fd, short events, void* arg)
{
	char msg[BUF_SIZE + 1];

	int ret = ::recv(fd, msg, BUF_SIZE, 0);
	if (ret <= 0)
	{
		perror("read fail ");
		exit(1);
	}

	int sockfd = *((int*)arg);

	//把终端的消息发送给服务器端
	//为了简单起见，不考虑写一半数据的情况
	::send(sockfd, msg, ret, 0);
}

static void read_cb(int fd, short events, void *arg)
{
	char msg[BUF_SIZE + 1];
// 	int rs = 1;
// 	int len = 0;
// 	while (rs)
// 	{
// 		len = ::recv(fd, msg, BUF_SIZE, 0);
// 		if (len < 0)
// 		{
// 			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
// 			// 在这里就当作是该次事件已处理处.
// 			if (errno == EAGAIN)
// 			{
// 				break;
// 			}
// 			else
// 			{
// 				return;
// 			}
// 		}
// 		else if (len == 0)
// 		{
// 			// 这里表示对端的socket已正常关闭.
// 		}
// 
// 		if (len == BUF_SIZE)
// 			rs = 1;   // 需要再次读取
// 		else
// 			rs = 0;
// 	}

	int len = ::recv(fd, msg, BUF_SIZE, 0);
	if (len == -1)
	{
		printf("network error.");
	}
	else if (len == 0)
	{
		printf("network dis.");
	}
	else
	{
		msg[len] = '\0';

		printf("recv %s from server\n", msg);
	}
}

static int connect_server(const char* server_ip, int port)
{
	int sockfd, save_errno;

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		return sockfd;
	}

	int status = ::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (status == -1)
	{
		save_errno = errno;
		evutil_closesocket(sockfd);
		errno = save_errno; //the close may be error
		return -1;
	}

	//evutil_make_socket_nonblocking(sockfd);

	return sockfd;
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

	int fd = connect_server("192.168.1.169", 9876);
	if (fd == -1)
	{
		perror("tcp_connect error ");
		return -1;
	}

	printf("connect to server successful\n");

	struct event_base* base = event_base_new();

	struct event *ev_sockfd = event_new(base, fd, EV_READ | EV_PERSIST, read_cb, nullptr);
	event_add(ev_sockfd, nullptr);

	event_base_dispatch(base);

	printf("finished \n");

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

#endif

