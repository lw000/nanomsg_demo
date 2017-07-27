// main.cpp : 定义控制台应用程序的入口点。
//

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

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

#include "command.h"
#include "platform.pb.h"

#include "http_server.h"

#include "PlatformServer.h"

#include "client_main.h"

#include "cmdline.h"
#include "libproperties.h"

#include "lwutil.h"
#include "UserMgr.h"

#include "..\libcrossLog\FastLog.h"

using namespace LW;

SocketServer __g_serv;
UserMgr __g_umgr;

std::string __s_center_server_addr;
std::string __s_center_server_port("19800");

static void _start_cb(int what)
{
	printf("RPC服务启动完成 [%d]！\n", __g_serv.getPort());

	__connect_center_server(__s_center_server_addr.c_str(), __s_center_server_port.c_str());
}

static void _add_user_thread()
{
	srand(time(NULL));
	while (1)
	{
		for (int i = 0; i < 10; i++)
		{
			USER_INFO info;
			info.id = i + rand() + 1000;
			__g_umgr.addUser(&info);
		}
// 		lw_sleep(1);
	}
}

static void _remove_user_thread()
{
	while (1)
	{
		__g_umgr.removeUserTest();
// 		lw_sleep(1);
	}
}

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

	lw_socket_init();

	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
#ifdef WIN32
	evthread_use_windows_threads();
#endif

	event_enable_debug_mode();

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

	hn_start_fastlog();

// 	std::thread a(_add_user_thread);
// 	std::thread b(_remove_user_thread);
// 	a.detach();
// 	b.detach();

	do 
	{
		std::string config(argv[1]);
		transform(config.begin(), config.end(), config.begin(), ::tolower);
		config = config.substr(config.size() - 4, 4);
		if (config.compare(".xml") != 0)
		{
			break;
		}

		Properties Pro;
		if (Pro.loadFromXML(argv[1]))
		{		
			__s_center_server_addr = Pro.getProperty("center_server_addr", "");
			if (__s_center_server_addr.empty())
			{
				break;
			}

			__s_center_server_port = Pro.getProperty("center_server_port", "19800");
			
			std::string sport = Pro.getProperty("port", "19901");
			lw_int32 port = std::atoi(sport.c_str());

			if (__g_serv.create(port, new PlatformServerHandler()) == 0)
			{
				__g_serv.run(_start_cb);
			}

			while (1) { lw_sleep(1); }
		}
		else
		{
			std::cout << "falue" << std::endl;
		}

	} while (0);

	lw_socket_clean();

	return 0;
}

