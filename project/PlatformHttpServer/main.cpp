// PlatformHttpServer.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <signal.h>
#include <algorithm>

#include <event2/thread.h>

#include "libproperties.h"

#include "http_server_business.h"
#include "net.h"
#include "lwutil.h"

int main(int argc, char** argv)
{
	if (argc < 2) return 0;

	SocketInit s;

	//如果要启用IOCP，创建event_base之前，必须调用evthread_use_windows_threads()函数
#ifdef WIN32
	evthread_use_windows_threads();
#endif

	int hport = 9877;

	Properties Pro;
	if (Pro.loadFromXML(argv[1]))
	{
		std::string shport = Pro.getProperty("port", "9877");

		hport = std::atoi(shport.c_str());

		__create_http_service_business(hport);

		while (1) { lw_sleep(1); }
	}
	else
	{
		std::cout << "falue" << std::endl;
	}

	return 0;
}

