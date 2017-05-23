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

#include "Message.h"
#include "platform.pb.h"
#include "Server.h"
#include "NetCmd.h"

using namespace LW;

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	struct bufferevent *bev = (struct bufferevent *)userdata;
	switch (cmd)
	{
	case CMD_HEART_BEAT:
	{
		{
			platform::csc_msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf(" order: %d\n", msg.order());
		}
		
		{
			platform::csc_msg_heartbeat msg;
			msg.set_order(1);

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
		printf(" userid: %d\n", client_userinfo.userid());

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

	Server serv;
	serv.run(9876, on_socket_recv);

// 	std::thread t(run_server, 9876);
// 	t.join();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

