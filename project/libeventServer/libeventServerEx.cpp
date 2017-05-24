// libevent-demo.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>
#include <vector>
#include <algorithm>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

#include "business.h"

#include <NetMessage.h>

#include "Message.h"
#include "platform.pb.h"
#include "Server.h"
#include "NetCmd.h"

using namespace LW;

Server __g_Serv;

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	struct bufferevent *bev = (struct bufferevent *)userdata;
	switch (cmd)
	{
	case CMD_HEART_BEAT:
	{
		//{
		//	platform::csc_msg_heartbeat msg;
		//	msg.ParseFromArray(buf, bufsize);
		//	printf(" order: %d\n", msg.time());
		//}
		//
		{
			platform::csc_msg_heartbeat msg;
			time_t t;
			t = time(NULL);
			//t = (time_t)ntohl(create_time);
			msg.set_time(t);

			lw_int32 len = (lw_int32)msg.ByteSizeLong();
			lw_char8 s[256] = { 0 };
			lw_bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				Server::sharedInstance()->sendData(bev, CMD_HEART_BEAT, s, len);
			}
		}

	} break;
	case CMD_PLATFORM_CS_USERINFO:
	{
		platform::sc_msg_request_userinfo client_userinfo;
		client_userinfo.ParseFromArray(buf, bufsize);
		//printf(" userid: %d\n", client_userinfo.userid());

		platform::sc_msg_userinfo userinfo;
		userinfo.set_userid(client_userinfo.userid());
		userinfo.set_age(30);
		userinfo.set_sex(1);
		userinfo.set_name("liwei");
		userinfo.set_address("guangdong shenzhen");

		char s[256] = { 0 };
		bool ret = userinfo.SerializePartialToArray(s, sizeof(s));

		Server::sharedInstance()->sendData(bev, CMD_PLATFORM_SC_USERINFO, s, strlen(s));

	} break;
	default:
		break;
	}
}

int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	WSADATA WSAData;
	int ret;
	if (ret = WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cout << "can not initilize winsock.dll" << std::endl;
		std::cout << "error code:" << WSAGetLastError() << std::endl;
		return 1;
	}
#endif
	
	/*int create_times = 10000000;
	{
		clock_t t = clock();
		for (size_t i = 0; i < create_times; i++)
		{
			NetMessage* msg = NetMessage::createNetMessage();
			if (nullptr != msg)
			{
				NetMessage::releaseNetMessage(msg);
			}
		}
		clock_t t1 = clock();
		printf("NetMessage create[%d] : %f \n", create_times, ((double)t1 - t) / CLOCKS_PER_SEC);
	}*/

	__g_Serv.init();

	__g_Serv.run(9876, on_socket_recv);

	__g_Serv.unInit();

// 	std::thread t(run_server, 9876);
// 	t.join();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

