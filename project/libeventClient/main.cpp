// libeventClient.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>

#include<event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/util.h>

#include "business.h"

#include "NetCmd.h"
#include "Message.h"
#include "platform.pb.h"
#include <thread>

using namespace LW;


#ifdef _WIN32
#define SLEEP(seconds) SleepEx(seconds * 1000, 1);
#else
#define SLEEP(seconds) sleep(seconds);
#endif


#define BUF_SIZE	1024

static void read_cb(struct bufferevent* bev, void* arg);
static void event_cb(struct bufferevent *bev, short event, void *arg);
static void time_cb(evutil_socket_t fd, short event, void *arg);

static lw_int32 send_socket_data(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

struct CLIENT
{
	struct event timer;
	struct bufferevent* bev;
	bool t;
};

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	struct bufferevent *bev = (struct bufferevent *)userdata;
	switch (cmd)
	{
	case CMD_PLATFORM_SC_USERINFO:
	{
		platform::sc_msg_userinfo userinfo;
		userinfo.ParseFromArray(buf, bufsize);
		printf(" userid: %d age:%d sex:%d name:%s address:%s\n", userinfo.userid(),
			userinfo.age(), userinfo.sex(), userinfo.name().c_str(), userinfo.address().c_str());
	}break;
	default:
		break;
	}
}

static lw_int32 send_socket_data(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	lw_int32 result = 0;
	{
		LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);
		result = bufferevent_write(bev, p->buf, p->size);
		lw_free_net_message(p);
	}
	return result;
}

static void time_cb(evutil_socket_t fd, short event, void *arg)
{
	struct CLIENT* client = (CLIENT*)arg;
	if (client->t)
	{
		// 设置定时器回调函数 
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		event_add(&client->timer, &tv);

		{
			platform::sc_msg_request_userinfo msg;
			msg.set_userid(400001);

			int len = (int)msg.ByteSizeLong();
			char s[256] = { 0 };
			bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				send_socket_data(client->bev, CMD_PLATFORM_CS_USERINFO, s, len);
			}
		}
	}
}

static void read_cb(struct bufferevent* bev, void* arg)
{
	struct evbuffer *input = bufferevent_get_input(bev);
	size_t input_len = evbuffer_get_length(input);

	{
		char *read_buf = (char*)malloc(input_len);

		size_t read_len = bufferevent_read(bev, read_buf, input_len);

		if (lw_parse_socket_data(read_buf, read_len, on_socket_recv, bev) == 0)
		{

		}

		free(read_buf);
	}
}

static void event_cb(struct bufferevent *bev, short event, void *arg)
{
	struct CLIENT* item = (CLIENT*)arg;

	if (event & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		printf("some other error\n");
	}
	else if (event & BEV_EVENT_TIMEOUT)
	{
		printf("timeout ...\n");
	}
	else if (event & BEV_EVENT_CONNECTED)
	{
		item->t = true;
		printf("conneted ...\n");

		return;
	}

	//这将自动close套接字和free读写缓冲区  
	bufferevent_free(bev);
	item->t = false;
}

void runClient()
{
	struct CLIENT client = { 0 };

	struct event_base *base = event_base_new();
	if (NULL != base)
	{
		client.bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(9876);
		addr.sin_addr.s_addr = inet_addr("192.168.1.169");

		bufferevent_setcb(client.bev, read_cb, NULL, event_cb, (void*)&client);

		int con = bufferevent_socket_connect(client.bev, (struct sockaddr *)&addr, sizeof(addr));
		if (con >= 0)
		{
			bufferevent_enable(client.bev, EV_READ | EV_PERSIST);

			// 设置定时器 
			event_assign(&client.timer, base, -1, 0, time_cb, (void*)&client);
			struct timeval tv;
			evutil_timerclear(&tv);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			event_add(&client.timer, &tv);

			event_base_dispatch(base);
			event_base_free(base);
		}
		else
		{
			bufferevent_free(client.bev);
		}
	}
}

int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	WSADATA WSAData;
	if (int ret = WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cout << "Can not initilize winsock.dll" << std::endl;
		std::cout << "Error Code:" << WSAGetLastError() << std::endl;
		return 1;
	}
#endif
	for (size_t i = 0; i < 500; i++)
	{
		std::thread t(runClient);
		t.detach();
		SLEEP(0.1);
	}

	int ch = getchar();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

