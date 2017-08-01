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

extern "C"
{
	#include "md5.h"
	#include "base64.h"
}

using namespace LW;

SocketServer __g_serv;

void base64_test(char * s)
{
	char*b;
	int l = base64Encode((unsigned char*)s, strlen(s), &b);
	{
		unsigned char*b1;
		int ll = base64Decode((unsigned char*)b, l, &b1);
		b1[ll] = 0;
		free(b1);
	}
	
	free(b);
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

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

	lw_socket_init();

	md5_test();

	base64_test("aaaaaaaaaaaaaaaaaaaaaaaaaa");

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

			if (__g_serv.create(port, new ServerHandler()) == 0)
			{
				__g_serv.run([](int what) {
					printf("中心服务器服务启动完成 [%d]！\n", __g_serv.getPort());
				});
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

