#include "http_server.h"

#include <stdio.h>
#include <signal.h>
#include <string>
#include <vector>
#include <thread>
#include <unordered_map>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/event_struct.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/util.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "lwutil.h"
#include <memory>

void lw_http_send_reply(struct evhttp_request * req, const char* what)
{
	struct evbuffer *buf = evbuffer_new();
	// 	evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
	// 	evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
	// 	evhttp_add_header(req->output_headers, "Connection", "close");
	evbuffer_add_printf(buf, what);
	evhttp_send_reply(req, HTTP_OK, "Client", buf);
	evbuffer_free(buf);
}

static void __default_handler(struct evhttp_request *req, void *arg)
{
	HTTP_METHOD_SIGNATURE* signature = (HTTP_METHOD_SIGNATURE*)arg;
	{
		signature->_cb(req);
	}
}

static void __method_handler(struct evhttp_request *req, void *arg)
{
	HTTP_METHOD_SIGNATURE* signature = (HTTP_METHOD_SIGNATURE*)arg;
	
	switch (evhttp_request_get_command(req))
	{
	case EVHTTP_REQ_GET:
	case EVHTTP_REQ_POST:
	{
		signature->_cb(req);
	}
	default:
		lw_http_send_reply(req, "{\"code\":0,\"what\":\"The requested resource does not support http method 'POST' OR 'GET'.""}");
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer()
{
	this->_base = nullptr;
	this->_htpServ = nullptr;
}

HttpServer::~HttpServer()
{

}

lw_int32 HttpServer::create(const char* addr, lw_uint32 port)
{
	this->_addr = addr;
	this->port = port;

	struct evhttp_bound_socket *handle = NULL;

	struct event_config *cfg = event_config_new();
	//event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg)
	{
		this->_base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}

	this->_htpServ = evhttp_new(this->_base);
	if (!this->_htpServ)
	{
		fprintf(stderr, "couldn't?create?evhttp.?Exiting.\n");
		event_base_free(this->_base);
		return -1;
	}

	handle = evhttp_bind_socket_with_handle(this->_htpServ, this->_addr.c_str(), port);
	if (!handle)
	{
		fprintf(stderr, "couldn't?bind?to?port?%d.?exiting.\n", (int)port);
		evhttp_free(this->_htpServ);
		event_base_free(this->_base);
		return -1;
	}

	// 设置超时
	evhttp_set_timeout(this->_htpServ, 30);

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

void HttpServer::run()
{
	std::thread t(std::bind(&HttpServer::__run, this));
	t.detach();
}

// 设置回调 
void HttpServer::gen(HTTP_CB cb)
{
	HTTP_METHOD_SIGNATURE* signature = new HTTP_METHOD_SIGNATURE;
	signature->_signature = "__evhttp_set_gencb__";
	signature->_cmd = 1;
	signature->_cb = cb;
	evhttp_set_gencb(this->_htpServ, __default_handler, signature);
}

void HttpServer::get(const char * path, HTTP_CB cb)
{
	HTTP_METHOD_SIGNATURE* signature = new HTTP_METHOD_SIGNATURE;
	signature->_signature = path;
	signature->_cmd = 1;
	signature->_cb = cb;
	int r = evhttp_set_cb(this->_htpServ, path, __method_handler, signature);
	if (r == 0)
	{
		iterator iter = this->_method.find(path);
		if (iter == _method.end())
		{
			this->_method.insert(std::pair<std::string, HTTP_METHOD_SIGNATURE*>(path, signature));
		}
		else
		{
			iter->second = signature;
		}
	}
}

void HttpServer::post(const char * path, HTTP_CB cb)
{
	HTTP_METHOD_SIGNATURE* signature = new HTTP_METHOD_SIGNATURE;
	signature->_signature = path;
	signature->_cmd = 1;
	signature->_cb = cb;
	int r = evhttp_set_cb(this->_htpServ, path, __method_handler, signature);
	if (r == 0)
	{
		iterator iter = this->_method.find(path);
		if (iter == _method.end())
		{
			this->_method.insert(std::pair<std::string, HTTP_METHOD_SIGNATURE*>(path, signature));
		}
		else
		{
			iter->second = signature;
		}
	}
}

void HttpServer::__run()
{
	int ret = event_base_dispatch(_base);

	evhttp_free(this->_htpServ);

	event_base_free(this->_base);


	this->_htpServ = NULL;
	this->_base = NULL;
}