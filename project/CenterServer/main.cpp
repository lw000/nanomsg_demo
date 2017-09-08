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

#include "socket_server.h"

#include "libproperties.h"
#include "CenterServer.h"
#include "lw_util.h"
#include "lw_xxtea.h"
#include "lw_base64.h"

#include "FileUtils.h"

#include "socket_timer.h"
#include "socket_processor.h"

extern "C"
{
	#include "ext/md5.h"
}

#include "log4z.h"
#include "FastLog.h"
#include "net.h"

void xxtea_test(char* s)
{
	XXTea tea("liwei");
	tea.encrypt((unsigned char*)s, strlen(s), [&tea](unsigned char* out, unsigned int len)
	{
		printf("xxtea encrypt: [%d] -> %s\n", len, out);
		tea.decrypt(out, len, [](unsigned char* out, unsigned int len)
		{
			printf("xxtea decrypt: [%d] -> %s\n", len, out);
		});
	});

	tea.encrypt("./CenterServer.h", "./CenterServer.hh");
	tea.decrypt("./CenterServer.hh", "./CenterServer.hhh");
}

void base64_test(char * s)
{
	Base64 b64;

	b64.encrypt((unsigned char*)s, strlen(s), [&b64](char* out, unsigned int len)
	{
		printf("base64 encrypt: [%d] -> %s\n", len, out);
		b64.decrypt((unsigned char*)out, len, [](unsigned char* out, unsigned int len)
		{
			printf("base64 decrypt: [%d] -> %s\n", len, out);
		});
	});

	b64.encrypt("./CenterServer.cpp", [](char* out, unsigned int len)
	{
		printf("base64 encrypt file: [%d] -> %s\n", len, out);
		FileUtils::getInstance()->setDataToFile("./CenterServer.cppp", (unsigned char*)out, len);
	});

	b64.decrypt("./CenterServer.cppp", [](unsigned char* out, unsigned int len)
	{
		printf("base64 encrypt file: [%d] -> %s\n", len, out);
		FileUtils::getInstance()->setDataToFile("./CenterServer.cpppp", (unsigned char*)out, len);
	});
}

int md5_test()
{
	char *md5sum = NULL, buf[65];

	char* s = "11111111111111111111111111111111";
	char* s1 = "./CenterServer.h";
	if (strlen(s) != 32)
		fprintf(stderr, "WARNING: MD5 hash size is wrong.\n");

	md5sum = MD5File(s1, buf);
	if (!md5sum)
	{
		perror("Could not obtain MD5 sum");
		return -1;
	}

	if (!strcasecmp(md5sum, s))
	{
		fprintf(stderr, "%s: OK\n", s1);
	}
	else
	{
		fprintf(stderr, "%s: FAILED.  Checksum is %s\n", s1, md5sum);
	}

	char buf1[64 + 1];
	std::string sss = MD5Data("11111111111111", strlen("11111111111111"), buf1);

	return 0;
}

static Timer			__g_timer1;
static SocketProcessor	__g_processor1;

static Timer			__g_timer2;
static SocketProcessor	__g_processor2;

static SocketServer		__g_serv;

static void start_cb(lw_int32 what)
{
	char s[512];
	sprintf(s, "中心服务器服务启动完成 [port: %d]", __g_serv.getPort());
	LOGD(s);
}

static void test1()
{
	int exec_times = 1000;
	
	__g_processor1.create(true, nullptr);

	__g_timer1.create(&__g_processor1);
	for (int i = 1; i < exec_times; i++)
	{
		__g_timer1.start(i, 1000, [](int tid, unsigned int tms) -> bool
		{
			char s[512];
			sprintf(s, "timer tid = [%d], time = [%f]", tid, double(tms) / 1000.0);
			LOGD(s);

			return true;
		});
	}

	__g_timer2.create(&__g_processor1);
	for (int i = 1; i < exec_times; i++)
	{
		__g_timer2.start(i, 1000, [](int tid, unsigned int tms) -> bool
		{
			char s[512];
			sprintf(s, "timer tid = [%d], time = [%f]", tid, double(tms) / 1000.0);
			LOGD(s);

			return true;
		});
	}

	__g_processor1.dispatch();

	__g_processor1.destroy();

	//for (int i = 1; i < exec_times; i++)
	//{
	//	__g_timer1.kill(i);
	//}
}

static void test2()
{
	int exec_times = 1000;
	__g_processor2.create(true, nullptr);

	__g_timer2.create(&__g_processor2);
	for (int i = 1; i < exec_times; i++)
	{
		__g_timer2.start(i, 1000, [](int tid, unsigned int tms) -> bool
		{
			char s[512];
			sprintf(s, "timer1 tid = [%d], time = [%f]", tid, double(tms) / 1000.0);
			LOGD(s);

			return true;
		});
	}

	__g_processor2.dispatch();

	//for (int i = 1; i < exec_times; i++)
	//{
	//	__g_timer2.kill(i);
	//}

	__g_processor2.destroy();
}

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

	SocketInit sinit;

// 	md5_test();
// 
// 	base64_test("aaaaaaaaaaaaaaaaaaaaaaaaaa");
// 	xxtea_test("aaaaaaaaaaaaaaaaaaaaaaaaaa");


	SocketProcessor::processorUseThreads();

	std::cout << __g_processor1 << std::endl;
	std::cout << __g_processor2 << std::endl;

	hn_start_fastlog();

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

			if (__g_serv.create(new ServerHandler()))
			{
				__g_serv.run(port, start_cb);
			}

			//std::thread a(test1);
			//std::thread b(test2);
			//a.detach();
			//b.detach();
		}
		else
		{
			std::cout << "falue" << std::endl;
		}

		while (1) { lw_sleep(1); }

	} while (0);

	return 0;
}

