
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

#include "business.h"

#include "Message.h"
#include "platform.pb.h"

#include "nn.hpp"

using namespace LW;

#define NODE0 "node0"
#define NODE1 "node1"

#define SOCKET_ADDR "ipc:///tmp/pair.ipc"

static lw_int32 send_data(nn::socket& s, lw_int32 cmd, void* object, lw_int32 objectSize);
static lw_int32 recv_data(nn::socket& s);
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

static lw_int32 send_data(nn::socket& s, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);

	lw_int32 result = s.send(p->buf, p->size, 0);

	lw_free_net_message(p);

	return result;
}

static lw_int32 recv_data(nn::socket& s)
{
	char buf[1024];
	lw_int32 result = s.recv(buf, sizeof(buf), 0); /*NN_MSG*/
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

static int on_pair_data(nn::socket& s)
{
	int to = 100;
	s.setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to));
	while (1)
	{
		{
			recv_data(s);
		}
		
		//lw_sleep(1);

		{
			platform::sc_msg_userinfo msg;
			msg.set_age(30);
			msg.set_sex(1);
			msg.set_name("liwei");
			msg.set_address("guangdong shenzhen nanshan guangdong shenzhen nanshan guangdong shenzhen nanshan");

			char buf[256] = { 0 };
			bool ret = msg.SerializePartialToArray(buf, sizeof(buf));

			send_data(s, 10000, buf, strlen(buf));
		}		
	}
}

static int server_node(const char *url)
{
	nn::socket s(AF_SP, NN_PAIR);
	assert(s.bind(url) >= 0);

	on_pair_data(s);

	s.shutdown(0);

	return 0;
}

static int client_node(const char *url)
{
	nn::socket s(AF_SP, NN_PAIR);
	assert(s.connect(url) >= 0);

	on_pair_data(s);

	s.shutdown(0);

	return 0;
}

#if 0

int main(int argc, char **argv)
{
	if (argc < 2) return 0;

	if (strncmp(NODE0, argv[1], strlen(NODE0)) == 0) {
		return server_node(SOCKET_ADDR);
	}

	if (strncmp(NODE1, argv[1], strlen(NODE1)) == 0) {
		return client_node(SOCKET_ADDR);
	}

	fprintf(stderr, "Usage: pair %s|%s <ARG> ...\n", NODE0, NODE1);
	return 1;
}

#endif
