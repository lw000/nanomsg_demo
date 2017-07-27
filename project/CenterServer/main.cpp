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
#include <event2/thread.h>
#include <event2/util.h>

#include "business.h"

#include "socket_server.h"

#include "libproperties.h"
#include "CenterServer.h"
#include "lwutil.h"

using namespace LW;

SocketServer __g_serv;

static void _start_cb(int what)
{
	printf("中心服务器服务启动完成 [%d]！\n", __g_serv.getPort());
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
			std::string sport = Pro.getProperty("port", "19800");

			lw_int32 port = std::atoi(sport.c_str());

			if (__g_serv.create(port, new CenterServerHandler()) == 0)
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

