// SQLServer.cpp : Defines the entry point for the console application.
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

#include "libproperties.h"

#include "lwutil.h"

#include "SQLMgr.h"
#include "SQLTable.h"

#include "pthread.h"


using namespace LW;

SocketServer __g_serv;
SQLMgr		 __g_sqlmgr;

struct st_worker_thread_param {
	SQLMgr *sqlMgr;
};

static void* thread_one_action(void *arg);

static void* thread_one_action(void *arg)
{
	struct st_worker_thread_param *handles = (struct st_worker_thread_param*) arg;

	std::cout << "Thread 1: driver->threadInit()" << std::endl;

	handles->sqlMgr->getDriver()->threadInit();

	clock_t t = clock();
	for (size_t i = 0; i < 10; i++)
	{
		SQLTableQuotation quotation(handles->sqlMgr);
		quotation.reset();
		quotation.createStatement();
		quotation.executeQuery("SELECT * FROM quotation;");
	}
	clock_t t1 = clock();
	printf("times: %f \n", ((double)t1 - t) / CLOCKS_PER_SEC);

	std::cout << "Thread 1: driver->threadEnd()" << std::endl;

	handles->sqlMgr->getDriver()->threadEnd();

	return NULL;
}

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

static void _start_cb(int what)
{
	printf("数据库服务启动完成 [%d]！\n", __g_serv.getPort());
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
	
#ifdef WIN32
	evthread_use_windows_threads();
#endif

	event_enable_debug_mode();

	{
		do 
		{
#if 0
			if (!__g_sqlmgr.connect("172.16.1.61", "lw", "qazxsw123", "app_project")) break;
#else
			if (!__g_sqlmgr.connect("tcp://172.16.1.61:3306", "lw", "qazxsw123")) break;
			__g_sqlmgr.useSchema("app_project");
#endif
			{
				SQLTableConfig config(&__g_sqlmgr);
				config.reset();
				config.createStatement();
				config.executeQuery("SELECT * FROM config;");
			}

			{
				SQLTableUser user(&__g_sqlmgr);
				{
					user.reset();
					user.createStatement();
					user.executeQuery("SELECT * FROM user;");
				}

				{
					user.reset();
					user.createStatement();
					user.executeQuery("SELECT * FROM user WHERE id=1;");
				}

				{
					user.reset();
					user.prepareStatement("SELECT * FROM user WHERE id=?;");
					user.setInt(1, 2);
					user.executeQuery();
				}

				{
					user.reset();
					user.prepareStatement("UPDATE user \
												SET sex = ? \
												WHERE name = ?; \
										");
					user.setInt(1, 1);
					user.setString(2, U2G("宋龙俊"));
					user.executeQuery();
				}
			}

			pthread_t thread_one;
	
			struct st_worker_thread_param *param = new st_worker_thread_param;
			param->sqlMgr = &__g_sqlmgr;

			/*std::thread st(thread_one_action, (void *)param);
			st.join();*/

			int status;
			status = pthread_create(&thread_one, NULL, thread_one_action, (void *)param);
			if (status != 0)
				throw std::runtime_error("Thread creation has failed");

			status = pthread_join(thread_one, NULL);
			if (status != 0)
				throw std::runtime_error("joining thread has failed");
			
			delete param;

		} while (0);
		
		int c = getchar();
	}

#if 0

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
			std::string sport = Pro.getProperty("port", "19902");
			lw_int32 port = std::atoi(sport.c_str());

			if (__g_serv.init() == 0)
			{
				__g_serv.run(port, _start_cb, on_socket_recv);
			}

			while (1) { lw_sleep(1); }
		}
		else
		{
			std::cout << "falue" << std::endl;
		}
	} while (0);

#endif

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}
