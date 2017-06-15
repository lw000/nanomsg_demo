// libeventClient.cpp : 定义控制台应用程序的入口点。
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

#include "socket_client.h"

using namespace LW;

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

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

void run_rpc_client(lw_int32 port)
{
	SocketClient* client = new SocketClient;
	if (client->init())
	{
		client->setRecvHook(on_socket_recv);

		client->setTimerHook([client](struct bufferevent* bev) -> bool
		{
			platform::sc_msg_request_userinfo msg;
			msg.set_userid(400001);

			int len = (int)msg.ByteSizeLong();
			char s[256] = { 0 };
			bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				client->send_data(CMD_PLATFORM_CS_USERINFO, s, len);

// 				lw_send_socket_data(CMD_PLATFORM_CS_USERINFO, s, len, [bev](LW_NET_MESSAGE * p) -> lw_int32
// 				{
// 					int c = bufferevent_write(bev, p->buf, p->size);
// 
// 					return true;
// 				});
			}

			return false;
		});

		int ret = client->start("127.0.0.1", port);
	}
}


// 当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig)
{
	switch (sig)
	{
	case SIGINT:
//	case SIGBREAK:
	case SIGABRT:
		//event_base_loopbreak(__http_base);
		break;
	}
}

int main(int argc, char** argv)
{
	signal(SIGINT | SIGABRT /* | SIGBREAK*/, signal_handler);

#if defined(WIN32) || defined(_WIN32)
	WSADATA WSAData;
	if (int ret = WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cout << "Can not initilize winsock.dll" << std::endl;
		std::cout << "Error Code:" << WSAGetLastError() << std::endl;
		return 1;
	}
#endif

	lw_int32 port = 0;
	lw_int32 rpc_times = 1;
	lw_int32 http_times = 1;

// 	cmdline::parser a;
// 	a.add<int>("http_client_times", 'C', "http_client_times ...");
// 	a.add<int>("rpc_exec_times", 'c', "rpc exec times ....");
// 	a.add<int>("port", 'r', "remote server port.", false, 9876, cmdline::range(9000, 65535));
// 
// 	a.parse_check(argc, argv);
// 
// 	if (a.exist("port"))
// 	{
// 		port = a.get<int>("port");
// 	}
// 
// 	if (a.exist("rpc_times"))
// 	{
// 		rpc_times = a.get<int>("rpc_times");
// 	}
// 
// 	if (a.exist("http_times"))
// 	{
// 		http_times = a.get<int>("http_times");
// 	}

	Properties Pro;
	if (Pro.loadFromXML(argv[1]))
	{
		std::string sport = Pro.getProperty("port", "9876");
		std::string shttp_times = Pro.getProperty("http_times", "1");
		std::string srpc_times = Pro.getProperty("rpc_times", "1");

		port = std::atoi(sport.c_str());
		rpc_times = std::atoi(srpc_times.c_str());
		http_times = std::atoi(shttp_times.c_str());
		
		for (size_t i = 0; i < rpc_times; i++)
		{
			std::thread t(run_rpc_client, port);
			t.detach();
			lw_sleep(0.1);
		}

		run_client_http(http_times);
	}
	else
	{
		std::cout << "falue" << std::endl;
	}

	int ch = getchar();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

