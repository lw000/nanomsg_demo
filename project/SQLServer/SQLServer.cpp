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

	SQLMgr::DriverThread driverThread(handles->sqlMgr);

	//handles->sqlMgr->getDriver()->threadInit();

	clock_t t = clock();
	for (size_t i = 0; i < 1; i++)
	{
		SQLTableQuotation sqlQuotation(handles->sqlMgr);
		sqlQuotation.reset();
		sqlQuotation.createStatement();
		sqlQuotation.executeQuery("SELECT * FROM quotation;", [&sqlQuotation](sql::ResultSet* res)
		{
			int row = res->rowsCount();

			while (res->next())
			{
				TableQuotation  quotation;
				quotation.name = U2G(res->getString("name").c_str());
				quotation.sale_name = U2G(res->getString("sale_name").c_str());
				quotation.quotation_number = U2G(res->getString("quotation_number").c_str());
				quotation.create_time = U2G(res->getString("create_time").c_str());

				quotation.print();
				sqlQuotation.vtQuotation.push_back(quotation);
			}
		},
			[](const std::string & error)
		{
			printf("# error: %s", error.c_str());
		});
	}
	clock_t t1 = clock();
	printf("times: %f \n", ((double)t1 - t) / CLOCKS_PER_SEC);

	//handles->sqlMgr->getDriver()->threadEnd();

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
				SQLTableConfig sqlConfig(&__g_sqlmgr);
				sqlConfig.reset();
				sqlConfig.createStatement();
				sqlConfig.executeQuery("SELECT * FROM config;", [&sqlConfig](sql::ResultSet* res)
				{
					uint32_t  c0 = res->findColumn("proportion"); 
					uint32_t  c1 = res->findColumn("meal_supplement");
					uint32_t  c2 = res->findColumn("overtime");
					uint32_t  c3 = res->findColumn("finance");
					uint32_t  c4 = res->findColumn("work_day");

					int row = res->rowsCount();

					while (res->next())
					{
						sqlConfig.config.proportion = res->getDouble(1);
						sqlConfig.config.meal_supplement = res->getDouble(2);
						sqlConfig.config.overtime = res->getDouble(3);
						sqlConfig.config.finance = res->getDouble(4);
						sqlConfig.config.work_day = res->getInt(5);
						sqlConfig.config.print();
					}
				},
					[](const std::string & error)
				{

				});
			}

			{
				SQLTableUser user(&__g_sqlmgr);
				{
					user.reset();
					user.createStatement();
					user.executeQuery("SELECT * FROM user;", [](sql::ResultSet* res)
					{

					},
						[](const std::string & error)
					{

					});
				}

				{
					user.reset();
					user.createStatement();
					user.executeQuery("SELECT * FROM user WHERE id=1;", [](sql::ResultSet* res)
					{

					},
					[](const std::string & error)
					{
						printf("# error: %s", error.c_str());
					});
				}

				{
					user.reset();
					sql::PreparedStatement* pstmt = user.prepareStatement("SELECT * FROM user WHERE id=?;");
					pstmt->setInt(1, 2);
					user.executeQuery([](sql::ResultSet* res)
					{

					},
						[](const std::string & error)
					{
						printf("# error: %s", error.c_str());
					});
				}

				{
					user.reset();
					sql::PreparedStatement* pstmt = user.prepareStatement("UPDATE user \
												SET sex = ? \
												WHERE name = ?;");

					pstmt->setInt(1, 1);
					pstmt->setString(2, U2G("宋龙俊"));
					user.executeQuery([](sql::ResultSet* res)
					{

					},
						[](const std::string & error)
					{
						printf("# error: %s", error.c_str());
					});
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
		event_enable_debug_mode();

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
