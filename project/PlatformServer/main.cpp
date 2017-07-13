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

#include "socket_session.h"
#include "socket_server.h"

#include "client_main.h"

#include "cmdline.h"
#include "libproperties.h"

#include "lwutil.h"

using namespace LW;

SocketServer __g_serv;
FILE * logfile;

std::string __s_center_server_addr;
std::string __s_center_server_port("19800");

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	SocketSession *session = (SocketSession *)userdata;
	switch (cmd)
	{
	case cmd_heart_beat:
	{
		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));

		lw_int32 len = (lw_int32)msg.ByteSizeLong();
		lw_char8 s[256] = { 0 };
		lw_bool ret = msg.SerializeToArray(s, len);
		if (ret)
		{
			session->sendData(cmd_heart_beat, s, len);
		}
	} break;
	case cmd_platform_cs_userinfo:
	{
		platform::msg_userinfo_request client_userinfo;
		client_userinfo.ParseFromArray(buf, bufsize);
		//printf(" userid: %d\n", client_userinfo.userid());

		platform::msg_userinfo_reponse userinfo;
		userinfo.set_uid(client_userinfo.uid());
		userinfo.set_age(30);
		userinfo.set_sex(1);
		userinfo.set_name("liwei");
		userinfo.set_address("guangdong");

		char s[256] = { 0 };
		bool ret = userinfo.SerializePartialToArray(s, sizeof(s));
		session->sendData(cmd_platform_sc_userinfo, s, strlen(s));

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
	printf("RPC服务启动完成 [%d]！\n", __g_serv.getPort());

	__connect_center_server(__s_center_server_addr.c_str(), __s_center_server_port.c_str());
}

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

	lw_socket_init();

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
			__s_center_server_addr = Pro.getProperty("center_server_addr", "");
			if (__s_center_server_addr.empty())
			{
				break;
			}

			__s_center_server_port = Pro.getProperty("center_server_port", "19800");
			
			std::string sport = Pro.getProperty("port", "19901");
			lw_int32 port = std::atoi(sport.c_str());

			if (__g_serv.create(port) == 0)
			{
				__g_serv.run(_start_cb, on_socket_recv);
			}

			while (1) { lw_sleep(1); }
		}
		else
		{
			std::cout << "falue" << std::endl;
		}

	} while (0);

	lw_socket_celan();

	return 0;
}

