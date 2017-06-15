#include "http_server.h"

#include <stdio.h>
#include <signal.h>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "lwutil.h"

#define MYHTTPD_SIGNATURE   "lwstar v2.0.1"

//当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
// void signal_handler(evutil_socket_t fd, short event, void *user_data)
// {
// 	switch (sig)
// 	{
// 	case SIGINT:
// 	case SIGILL:
// 	case SIGFPE:
// 	case SIGSEGV:
// 	case SIGTERM:
// 	case SIGBREAK:
// 	case SIGABRT:
// 		event_base_loopbreak(__http_base);
// 		break;
// 	}
// }

HttpServer::HttpServer()
{

	
}

HttpServer::~HttpServer()
{
}

lw_int32 HttpServer::init(const char* addr, lw_uint32 port)
{
	this->_addr = addr;
	this->port = port;

	char uri_root[512];

	struct evhttp_bound_socket *handle = NULL;

	struct event_config *cfg = event_config_new();
	//event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg)
	{
		_base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}

	_httpServ = evhttp_new(_base);
	if (!_httpServ)
	{
		fprintf(stderr, "couldn't?create?evhttp.?Exiting.\n");
		event_base_free(_base);
		return -1;
	}

	handle = evhttp_bind_socket_with_handle(_httpServ, this->_addr.c_str(), port);
	if (!handle)
	{
		fprintf(stderr, "couldn't?bind?to?port?%d.?exiting.\n", (int)port);
		evhttp_free(_httpServ);
		event_base_free(_base);
		return -1;
	}

	// 设置超时
	evhttp_set_timeout(_httpServ, 60);

	/* Extract and display the address we're listening on. */
	{
		struct sockaddr_storage ss;
		evutil_socket_t fd;
		ev_socklen_t socklen = sizeof(ss);
		char addrbuf[128];
		void *inaddr;
		const char *addr;
		int got_port = -1;
		fd = evhttp_bound_socket_get_fd(handle);
		memset(&ss, 0, sizeof(ss));
		do
		{
			if (getsockname(fd, (struct sockaddr *)&ss, &socklen))
			{
				perror("getsockname() failed");
				break;
			}

			if (ss.ss_family == AF_INET)
			{
				got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
				inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
			}
			else if (ss.ss_family == AF_INET6)
			{
				got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
				inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
			}
			else
			{
				fprintf(stderr, "weird address family %d\n", ss.ss_family);
				break;
			}

			addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf, sizeof(addrbuf));
			if (addr)
			{
				printf("HTTP服务启动完成 [http://%s:%d]\n", addr, got_port);
// 				evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d", addr, got_port);
			}
			else
			{
				fprintf(stderr, "evutil_inet_ntop failed\n");
				break;
			}

		} while (0);
	}

	return 0;
}

void HttpServer::start()
{
	std::thread t(std::bind(&HttpServer::__run, this));
	t.detach();
}

// 设置回调 
void HttpServer::set_http_gen_hook(LW_HTTP_CB cb, void * cb_arg)
{
	evhttp_set_gencb(this->_httpServ, cb, this);
}

void HttpServer::set_http_hook(const char *path, LW_HTTP_CB cb, void *cb_arg)
{
	evhttp_set_cb(this->_httpServ, path, cb, cb_arg);
}

void HttpServer::__run()
{
	int ret = event_base_dispatch(_base);

	evhttp_free(_httpServ);

	event_base_free(_base);


	_httpServ = NULL;
	_base = NULL;
}