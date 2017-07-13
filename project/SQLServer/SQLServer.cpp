// SQLServer.cpp : Defines the entry point for the console application.
//


#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#else
#include <uuid/uuid.h> 
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

#include "socket_server.h"

#include "libproperties.h"

#include "lwutil.h"

#include "SQLTable.h"

#include "pthread.h"

#include "FastLog.h"
#include "SQLConnPool.h"
#include "socket_session.h"

using namespace LW;

SocketServer __g_serv;

struct st_worker_thread_param {
	sql::Connection *conn;
};

static void* thread_one_action(void *arg);

static void* thread_one_action(void *arg)
{
	SQLConnPool::CONNECT conn;
	{
		clock_t t = clock();
		int times = 1;
		for (size_t i = 0; i < times; i++)
		{
			SQLTableQuotation sqlQuotation(&conn);
			{
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
						sqlQuotation.vtQuotation.push_back(quotation);
					}
				},
					[](const std::string & error)
				{
					printf("# error: %s", error.c_str());
				});
			}
		}
		clock_t t1 = clock();
		printf("select [%d] times: %f \n", times, ((double)t1 - t) / CLOCKS_PER_SEC);

	}
	
	{
		int times = 1000;
		clock_t t = clock();

		SQLTableQuotation sqlQuotation(&conn);
		sql::PreparedStatement* pstmt = sqlQuotation.prepareStatement("INSERT INTO quotation VALUES(?,?,?,?);");
		pstmt->setString(1, G2U("祈宜鸟直播 - 新增功能"));
		pstmt->setString(2, G2U("刘玉婷"));
		pstmt->setDateTime(4, G2U("2017-06-21 00:00:00"));

		for (int i = 0; i < times; i++)
		{
#ifdef _WIN32
#define GUID_LEN 64
			char buffer[GUID_LEN] = { 0 };
			GUID guid;

			if (CoCreateGuid(&guid))
			{
				fprintf(stderr, "create guid error\n");
			}

			_snprintf(buffer, sizeof(buffer),
				"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);
			pstmt->setString(3, G2U(buffer));
#undef GUID_LEN
#else
			uuid_t uu;
			int i;
			uuid_generate(uu);

			for (i = 0; i < 16; i++)
			{
				printf("%02X-", uu[i]);
			}
			printf("\n");
#endif // _WIN32

			sqlQuotation.executeQuery([&sqlQuotation](sql::ResultSet* res)
			{
				int row = res->rowsCount();
			},
				[](const std::string & error)
			{
				printf("# error: %s", error.c_str());
			});
		}

		clock_t t1 = clock();
		printf("select [%d] times: %f \n", times, ((double)t1 - t) / CLOCKS_PER_SEC);

	}
	return NULL;
}

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
	
	hn_start_fastlog();

#ifdef WIN32
	evthread_use_windows_threads();
#endif

	SQLConnPool* connPool = SQLConnPool::getInstance();

	{
		do
		{
#if 0
			int r = connPool->createSqlConnPool("172.16.1.61", "lw", "qazxsw123", "app_project", 10);
#else
			int r = connPool->createSqlConnPool("tcp://172.16.1.61:3306", "lw", "qazxsw123", "app_project", 10);
#endif
	
			if (r != 0) break;

			{
				sql::Connection* conn = connPool->getGetConnection();
				sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM config WHERE work_day=?;");
				stmt->setInt(1, 22);
				sql::ResultSet* res = stmt->executeQuery();
				int row = res->rowsCount();
				TableConfig config;
				while (res->next())
				{
					config.proportion = res->getDouble(1);
					config.meal_supplement = res->getDouble(2);
					config.overtime = res->getDouble(3);
					config.finance = res->getDouble(4);
					config.work_day = res->getInt(5);
					config.print();
				}
// 				SQLTableConfig sqlConfig(conn);
// 				sqlConfig.reset();
// 				sqlConfig.createStatement();
// 				sqlConfig.executeQuery("SELECT * FROM config;", [&sqlConfig](sql::ResultSet* res)
// 				{
// 					uint32_t  c0 = res->findColumn("proportion"); 
// 					uint32_t  c1 = res->findColumn("meal_supplement");
// 					uint32_t  c2 = res->findColumn("overtime");
// 					uint32_t  c3 = res->findColumn("finance");
// 					uint32_t  c4 = res->findColumn("work_day");
// 
// 					int row = res->rowsCount();
// 
// 					while (res->next())
// 					{
// 						sqlConfig.config.proportion = res->getDouble(1);
// 						sqlConfig.config.meal_supplement = res->getDouble(2);
// 						sqlConfig.config.overtime = res->getDouble(3);
// 						sqlConfig.config.finance = res->getDouble(4);
// 						sqlConfig.config.work_day = res->getInt(5);
// 						sqlConfig.config.print();
// 					}
// 				},
// 					[](const std::string & error)
// 				{
// 
// 				});
				connPool->recycleConnection(conn);
			}

			{
				sql::Connection* conn = connPool->getGetConnection();
				SQLTableUser user(conn);
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
					pstmt->setString(2, utf8_to_gbk("宋龙俊"));
					user.executeQuery([](sql::ResultSet* res)
					{

					},
						[](const std::string & error)
					{
						printf("# error: %s", error.c_str());
					});
				}

				connPool->recycleConnection(conn);
			}

			{
				for (int i = 0; i < 1; i++)
				{
					pthread_t thread_one;

					/*std::thread st(thread_one_action);
					st.join();*/

					int status;
					status = pthread_create(&thread_one, NULL, thread_one_action, NULL);
					if (status != 0)
						throw std::runtime_error("Thread creation has failed");

					status = pthread_detach(thread_one);
					if (status != 0)
						throw std::runtime_error("joining thread has failed");
				}

				//pthread_t thread_one;
				//struct st_worker_thread_param *param = new st_worker_thread_param;
				//param->conn = SQLConnPool::getInstance()->getGetConnection();

				///*std::thread st(thread_one_action, (void *)param);
				//st.join();*/

				//int status;
				//status = pthread_create(&thread_one, NULL, thread_one_action, (void *)param);
				//if (status != 0)
				//	throw std::runtime_error("Thread creation has failed");

				//status = pthread_join(thread_one, NULL);
				//if (status != 0)
				//	throw std::runtime_error("joining thread has failed");
				//delete param;
			}
			

		} while (0);
		
		int c = getchar();
		c = getchar();

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
