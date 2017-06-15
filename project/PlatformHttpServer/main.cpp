// PlatformHttpServer.cpp : Defines the entry point for the console application.
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

#include "libproperties.h"


#include "http_server.h"
#include "http_server_business.h"

#ifdef _WIN32
#define LW_SLEEP(seconds) SleepEx(seconds * 1000, 1);
#else
#define LW_SLEEP(seconds) sleep(seconds);
#endif

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

	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
#ifdef WIN32
	evthread_use_windows_threads();
#endif

	event_enable_debug_mode();

	int hport = 9877;

	Properties Pro;
	if (Pro.loadFromXML(argv[1]))
	{
		std::string shport = Pro.getProperty("port", "9877");

		hport = std::atoi(shport.c_str());

		__init_http_business(hport);

		while (1) { LW_SLEEP(1); }
	}
	else
	{
		std::cout << "falue" << std::endl;
	}

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

