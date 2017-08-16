// GameServer.cpp : 定义控制台应用程序的入口点。
//

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

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

#include "command.h"
#include "platform.pb.h"

#include "cmdline.h"
#include "lwutil.h"

#include "libproperties.h"
#include "client_http.h"

#include "GameServer.h"
#include "net.h"

using namespace LW;

void run_rpc_client(lw_int32 port)
{

}

// 当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig)
{
	switch (sig)
	{
	case SIGTERM:
	case SIGINT:	//通常是CTRL+C或者DELETE
	case SIGBREAK:
	case SIGABRT:	
		//event_base_loopbreak(__http_base);
		break;
	}
}

int main(int argc, char** argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGBREAK, signal_handler);
	signal(SIGTERM, signal_handler);

	SocketInit s;

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

	GameServer* gServer[1024];

	do 
	{
		std::string config(argv[1]);
		std::transform(config.begin(), config.end(), config.begin(), ::tolower);
		config = config.substr(config.size() - 4, 4);
		if (config.compare(".xml") != 0)
		{
			break;
		}

		Properties Pro;
		if (Pro.loadFromXML(argv[1]))
		{
			std::string sport = Pro.getProperty("port", "19801");
			std::string shttp_times = Pro.getProperty("http_times", "1");
			std::string srpc_times = Pro.getProperty("rpc_times", "1");

			port = std::atoi(sport.c_str());
			rpc_times = std::atoi(srpc_times.c_str());
			http_times = std::atoi(shttp_times.c_str());

			while (1)
			{
				for (size_t i = 0; i < rpc_times; i++)
				{
					DESK_INFO desk_info;
					desk_info.did = i;
					desk_info.max_usercount = 6;
					char buf[64];
					sprintf(buf, "测试[%d]", i);
					desk_info.name = std::string(buf);
					desk_info.rid = 0;
					desk_info.state = DESK_STATE_Empty;

					GameServer * serv = new GameServer(nullptr);
					if (serv->create(desk_info))
					{
						serv->start("127.0.0.1", port);
					}

					gServer[i] = serv;

					lw_sleep(0.1);
				}

				int c = getchar();
			}

//			run_client_http(http_times);
		}
		else
		{
			std::cout << "falue" << std::endl;
		}

		while (1)
		{
			lw_sleep(1);
		}
	} while (0);

	lw_socket_clean();

	return 0;
}

