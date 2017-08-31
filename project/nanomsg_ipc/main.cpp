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

#include "platform.pb.h"
#include "common_type.h"
#include "lwutil.h"
#include "command.h"

#include "nanomsg_socket.h"

using namespace LW;

#define NODE0 "node0"
#define NODE1 "node1"

#define SOCKET_ADDR "ipc:///tmp/pair.ipc"

class Server : public NanomsgSocket
{
public:
	Server() {
	}

	virtual ~Server() {
	}

public:
	virtual void onRecv(lw_int32 cmd, char* buf, lw_int32 bufsize) override {
		switch (cmd)
		{
		case cmd_heart_beat: {
			platform::msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf("heartBeat[%d]\n", msg.time());
		}break;
		default:
			break;
		}
	}
};

class Client : public NanomsgSocket
{
public:
	Client() {
	}

	~Client() {
	}

private:
	virtual void onRecv(lw_int32 cmd, char* buf, lw_int32 bufsize) override {
		switch (cmd)
		{
		case cmd_heart_beat: {
			platform::msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf("heartBeat[%d]\n", msg.time());
		}break;
		default:
			break;
		}
	}
};

Client __g_client;
Server __g_server;

static int on_pair_data(int sock, NanomsgSocket *c)
{
	int to = 100;
	assert(c->setsockopt(NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(int)) >= 0);
	while (1)
	{
		c->recv();

		lw_sleep(1);

		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));
		lw_int32 l = (lw_int32)msg.ByteSize();
		std::unique_ptr<char[]> s(new char[l + 1]);
		lw_bool ret = msg.SerializeToArray(s.get(), l);
		if (ret)
		{
			c->send(cmd_heart_beat, s.get(), l);
		}
	}
}

static int server_node(const char *url)
{
	int sock = __g_server.create(NN_PAIR);
	assert(sock >= 0);
	assert(__g_server.bind(url) >= 0);

	on_pair_data(sock, &__g_server);

	return nn_shutdown(sock, 0);
}

static int client_node(const char *url)
{
	int sock = __g_client.create(NN_PAIR);
	//int sock = nn_socket(AF_SP, NN_PAIR);
	assert(sock >= 0);
	assert(__g_client.connect(url) >= 0);

	on_pair_data(sock, &__g_client);

	return nn_shutdown(sock, 0);
}

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