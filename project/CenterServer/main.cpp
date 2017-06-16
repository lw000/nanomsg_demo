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
#include "Message.h"
#include "platform.pb.h"

#include "socket_server.h"

#include "cmdline.h"
#include "libproperties.h"

using namespace LW;

#ifdef _WIN32
#define LW_SLEEP(seconds) SleepEx(seconds * 1000, 1);
#else
#define LW_SLEEP(seconds) sleep(seconds);
#endif


SocketServer __g_serv;
FILE * logfile;

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	struct bufferevent *bev = (struct bufferevent *)userdata;
	switch (cmd)
	{
	case cmd_heart_beat:
	{
		platform::csc_msg_heartbeat msg;
		msg.set_time(time(NULL));

		lw_int32 len = (lw_int32)msg.ByteSizeLong();
		lw_char8 s[256] = { 0 };
		lw_bool ret = msg.SerializeToArray(s, len);
		if (ret)
		{
			__g_serv.sendData(bev, cmd_heart_beat, s, len);
		}
	} break;
	default:
		break;
	}
}

static void _write_to_file_cb(int severity, const char *msg)
{
	const char *s;
	if (!logfile)
		return;
	switch (severity)
	{
	case _EVENT_LOG_DEBUG: s = "debug"; break;
	case _EVENT_LOG_MSG:   s = "msg";   break;
	case _EVENT_LOG_WARN:  s = "warn";  break;
	case _EVENT_LOG_ERR:   s = "error"; break;
	default:               s = "?";     break; /* never reached */
	}
	fprintf(logfile, "[%s] %s\n", s, msg);
}

static void _event_fatal_cb(int err)
{

}

static void _start_cb(int what)
{
	printf("中心服务器服务启动完成 [%d]！\n", __g_serv.getPort());
}

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

#if defined(WIN32) || defined(_WIN32)
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(2, 2);
		int err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			printf("WSAStartup failed with error: %d\n", err);
			return 0;
		}
	}
#endif

	event_set_fatal_callback(_event_fatal_cb);

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

// 	cmdline::parser a;
// 	a.add<std::string>("config", 'c', "配置文件");
// 	a.add<int>("lport", 'l', "本地RPC服务器端口", false, __s_lport, cmdline::range(9000, 65535));
// 	a.add<int>("rport", 'r', "远程RPC服务器端口", false, __s_rport, cmdline::range(9000, 65535));
// 	a.parse_check(argc, argv);
// 
// 	std::string config;
// 	if (a.exist("config"))
// 	{
// 		config = a.get<std::string>("config");
// 	}
// 
// 	if (a.exist("lport"))
// 	{
// 		__s_lport = a.get<int>("lport");
// 	}
// 
// 	if (a.exist("rport"))
// 	{
// 		__s_rport = a.get<int>("rport");
// 	}

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

			if (__g_serv.init() == 0)
			{
				__g_serv.run(port, _start_cb, on_socket_recv);
			}

			while (1) { LW_SLEEP(1); }
		}
		else
		{
			std::cout << "falue" << std::endl;
		}

	} while (0);

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

