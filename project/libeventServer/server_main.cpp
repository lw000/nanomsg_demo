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

#include "NetCmd.h"
#include "Message.h"
#include "platform.pb.h"

#include "http_server.h"
#include "socket_server.h"
#include "rpc_server.h"
#include "client_main.h"

#include "cmdline.h"

using namespace LW;

SocketServer __g_serv;
FILE *logfile = NULL;

static lw_int32 __s_lport = 9876;
static lw_int32 __s_rport = 9876;
static lw_int32 __s_hport = 9878;

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
			msg.set_time(t);

			lw_int32 len = (lw_int32)msg.ByteSizeLong();
			lw_char8 s[256] = { 0 };
			lw_bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				__g_serv.sendData(bev, CMD_HEART_BEAT, s, len);
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
		userinfo.set_address("guangdong");

		char s[256] = { 0 };
		bool ret = userinfo.SerializePartialToArray(s, sizeof(s));

		__g_serv.sendData(bev, CMD_PLATFORM_SC_USERINFO, s, strlen(s));

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
	switch (severity) {
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
	printf("RPC服务启动完成！\n");

	run_rpc_client("127.0.0.1", __s_rport);
	run_http_server(__s_hport);
}

int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD(2, 2);

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			printf("WSAStartup failed with error: %d\n", err);
			return 0;
		}
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

	cmdline::parser a;
	a.add<int>("lport", 'l', "本地RPC服务器端口", false, __s_lport, cmdline::range(9000, 65535));
	a.add<int>("rport", 'r', "远程RPC服务器端口", false, __s_rport, cmdline::range(9000, 65535));
	a.add<int>("hport", 'h', "本地HTTP服务器端口", false, __s_hport, cmdline::range(9000, 65535));

	a.parse_check(argc, argv);

	if (a.exist("lport"))
	{
		__s_lport = a.get<int>("lport");
	}
	if (a.exist("rport"))
	{
		__s_rport = a.get<int>("rport");
	}if (a.exist("hport"))
	{
		__s_hport = a.get<int>("hport");
	}

	std::cout << "lport: " << __s_lport << std::endl;
	std::cout << "rport: " << __s_rport << std::endl;
	std::cout << "hport: " << __s_hport << std::endl;

	event_set_fatal_callback(_event_fatal_cb);

//	event_set_log_callback(_write_to_file_cb);

//	logfile = fopen("error.log", "w");

	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
#ifdef WIN32
	evthread_use_windows_threads();
#endif

	event_enable_debug_mode();

	if (__g_serv.init() == 0)
	{
		__g_serv.run(__s_lport, _start_cb, on_socket_recv);
	}

// 	fflush(logfile);
// 	fclose(logfile);

	while (1) {}

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

