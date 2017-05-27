
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <deque>
#include <mutex>

#include <iostream>

#include <nn.h>
#include <pair.h>

#ifdef _WIN32
#include <winsock.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "business.h"

#include "Message.h"
#include "platform.pb.h"

using namespace LW;

#define NODE0 "node0"
#define NODE1 "node1"

#define SOCKET_ADDR "ipc:///tmp/pair.ipc"

static lw_int32 send_socket_data(lw_int32 sock, lw_int32 cmd, void* object, lw_int32 objectSize);
static lw_int32 recv_socket_data(lw_int32 sock);
static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	switch (cmd)
	{
	case 10000:	//用户信息
	{
		platform::sc_msg_userinfo msg;
		msg.ParsePartialFromArray(buf, bufsize);
		printf("age: %d\n sex: %d\n name: %s\n address: %s\n",
			msg.age(), msg.sex(), msg.name().c_str(), msg.address().c_str());

	}break;
	case 10001:
	{

	}break;
	case 10002:
	{

	}break;
	default:
		break;
	}
}

static lw_int32 send_socket_data(lw_int32 sock, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);

	lw_int32 result = nn_send(sock, p->buf, p->size, 0);

	lw_free_net_message(p);

	return result;
}

static lw_int32 recv_socket_data(lw_int32 sock)
{
	char *buf = NULL;
	lw_int32 result = nn_recv(sock, &buf, NN_MSG, 0);
	if (result > 0)
	{
		lw_parse_socket_data(buf, result, on_socket_recv, NULL);

		nn_freemsg(buf);
	}
	else
	{

	}

	return result;
}

static int on_pair_data(int sock)
{
	int to = 100;
	assert(nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
	while (1)
	{
		{
			recv_socket_data(sock);
		}
		
		//SLEEP(1);

		platform::sc_msg_userinfo msg;
		msg.set_age(30);
		msg.set_sex(1);
		msg.set_name("liwei");
		msg.set_address("guangdong shenzhen nanshan guangdong shenzhen nanshan guangdong shenzhen nanshan");

		char s[256] = { 0 };
		bool ret = msg.SerializePartialToArray(s, sizeof(s));

		send_socket_data(sock, 10000, s, strlen(s));
	}
}

int server_node(const char *url)
{
	int sock = nn_socket(AF_SP, NN_PAIR);
	assert(sock >= 0);
	assert(nn_bind(sock, url) >= 0);

	on_pair_data(sock);

	return nn_shutdown(sock, 0);
}

int client_node(const char *url)
{
	int sock = nn_socket(AF_SP, NN_PAIR);
	assert(sock >= 0);
	assert(nn_connect(sock, url) >= 0);

	on_pair_data(sock);

	return nn_shutdown(sock, 0);
}

int main(int argc, char **argv)
{
	if (argc == 2 && strncmp(NODE0, argv[1], strlen(NODE0)) == 0) {
		return server_node(SOCKET_ADDR);
	}
	else if (argc == 2 && strncmp(NODE1, argv[1], strlen(NODE1)) == 0) {
		return client_node(SOCKET_ADDR);
	}
	else {
		fprintf(stderr, "Usage: pair %s|%s <ARG> ...\n", NODE0, NODE1);
		return 1;
	}
}