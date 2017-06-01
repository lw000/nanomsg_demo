// libeventClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "business.h"


#include "command.h"
#include "Message.h"
#include "platform.pb.h"

#include "cmdline.h"
#include "lwutil.h"

#include "libproperties.h"
#include "client_http.h"

using namespace LW;

#define BUF_SIZE	1024

static void read_cb(struct bufferevent* bev, void* arg);
static void event_cb(struct bufferevent *bev, short event, void *arg);
static void time_cb(evutil_socket_t fd, short event, void *arg);

static lw_int32 send_socket_data(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

struct CLIENT
{
	struct bufferevent* bev;
	bool live;
	struct event timer;
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
		printf("userid: %d age:%d sex:%d name:%s address:%s\n", userinfo.userid(),
			userinfo.age(), userinfo.sex(), userinfo.name().c_str(), userinfo.address().c_str());
	}break;
	default:
		break;
	}
}

static void time_cb(evutil_socket_t fd, short event, void *arg)
{
	struct CLIENT* client = (CLIENT*)arg;
	if (client->live)
	{
		// ���ö�ʱ���ص����� 
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
				lw_send_socket_data(CMD_PLATFORM_CS_USERINFO, s, len, [&client](LW_NET_MESSAGE * p) -> lw_int32
				{
					return bufferevent_write(client->bev, p->buf, p->size);
				});
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
		item->live = false;
		printf("some other error\n");
	}
	else if (event & BEV_EVENT_TIMEOUT)
	{
		printf("timeout ...\n");
	}
	else if (event & BEV_EVENT_CONNECTED)
	{
		item->live = true;
		printf("��������ӳɹ�\n");

		return;
	}

	//�⽫�Զ�close�׽��ֺ�free��д������  
	bufferevent_free(bev);

}

void runClient(lw_int32 port)
{
	struct CLIENT client = { 0 };

	struct event_base *base = event_base_new();
	if (NULL != base)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");

		client.bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		bufferevent_setcb(client.bev, read_cb, NULL, event_cb, (void*)&client);
		int con = bufferevent_socket_connect(client.bev, (struct sockaddr *)&addr, sizeof(addr));
		if (con >= 0)
		{
			bufferevent_enable(client.bev, EV_READ | EV_PERSIST);

			// ���ö�ʱ��
			event_assign(&client.timer, base, -1, 0, time_cb, (void*)&client); //EV_PERSIST

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

	lw_int32 rport = 0;
	lw_int32 exec_times = 1;
	lw_int32 rpc_client_times = 1;

	cmdline::parser a;
	a.add<int>("rpc_client_times", 'C', "rpc client times.");
	a.add<int>("exec_times", 'c', "http exec times.");
	a.add<int>("rport", 'r', "remote rpc server port.", false, 9876, cmdline::range(9000, 65535));

	a.parse_check(argc, argv);

	if (a.exist("rport"))
	{
		rport = a.get<int>("rport");
	}

	if (a.exist("exec_times"))
	{
		exec_times = a.get<int>("exec_times");
	}

	if (a.exist("rpc_client_times"))
	{
		rpc_client_times = a.get<int>("rpc_client_times");
	}

	Properties p;
	if (!p.loadFromXML("config.xml"))
	{
		std::cout << "falue" << std::endl;
	}
	else
	{
		std::cout << "use getProperty" << std::endl;
		std::cout << p.getProperty("lport", "9876") << std::endl;
		std::cout << p.getProperty("rport", "9876") << std::endl;
		std::cout << p.getProperty("hport", "9877") << std::endl;
		std::cout << p.getProperty("rmote_host", "127.0.0.1") << std::endl;
		p.clear();
	}

	for (size_t i = 0; i < rpc_client_times; i++)
	{
		std::thread t(runClient, rport);
		t.detach();
		lw_sleep(0.1);
	}

	client_http_main(exec_times);

	int ch = getchar();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}
