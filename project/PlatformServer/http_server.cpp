#include <stdio.h>
#include <signal.h>
#include <string>
#include <vector>

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

#include <thread>
#include <unordered_map>

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

#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"

#define POST_BUF_MAX			1024*4

static struct event_base* __http_base = NULL;

typedef void(*LW_HTTP_CB)(struct evhttp_request *, void *);

struct HTTP_BUSINESS_SIGNATURE
{
	char * _signature;
	char * _cmd;
	LW_HTTP_CB _cb;

public:
	HTTP_BUSINESS_SIGNATURE(char * signature, char* cmd, LW_HTTP_CB cb)
	{
		this->_signature = signature;
		this->_cmd = cmd;
		this->_cb = cb;
	}
};


static void http_default_handler(struct evhttp_request *req, void *arg);
static void post_login_cb(struct evhttp_request *req, void *arg);
static void get_add_cb(struct evhttp_request *req, void *arg);
static void get_sub_cb(struct evhttp_request *req, void *arg);

HTTP_BUSINESS_SIGNATURE httpgignature[] =
{
	HTTP_BUSINESS_SIGNATURE("/login", "POST", post_login_cb),
	HTTP_BUSINESS_SIGNATURE("/add", "GET", get_add_cb),
	HTTP_BUSINESS_SIGNATURE("/sub", "GET", get_sub_cb),
};

static void lw_http_send_reply(struct evhttp_request * req, const char* what)
{
	struct evbuffer *buf = evbuffer_new();
// 	evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
// 	evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
// 	evhttp_add_header(req->output_headers, "Connection", "close");
	evbuffer_add_printf(buf, what);
	evhttp_send_reply(req, HTTP_OK, "Client", buf);
	evbuffer_free(buf);
}

static const char* lw_get_command(struct evhttp_request * req)
{
	const char *cmd;
	switch (evhttp_request_get_command(req))
	{
	case EVHTTP_REQ_GET: cmd = "GET"; break;
	case EVHTTP_REQ_POST: cmd = "POST"; break;
	case EVHTTP_REQ_HEAD: cmd = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmd = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmd = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmd = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmd = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmd = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmd = "PATCH"; break;
	default: cmd = "unknown"; break;
	}

	return cmd;
}

static void http_default_handler(struct evhttp_request *req, void *arg)
{
	evhttp_send_error(req, HTTP_BADREQUEST, "sorry，你走错地方了！");
	return;
}

static void post_login_cb(struct evhttp_request *req, void *arg)
{
	if (evhttp_request_get_command(req) != EVHTTP_REQ_POST)
	{
		lw_http_send_reply(req, "{\"code\":0,\"what\":\"The requested resource does not support http method 'GET'.""}");
		return;
	}

	char buff[POST_BUF_MAX] = "\0";

	struct evbuffer *evbuf; 
	evbuf = evhttp_request_get_input_buffer(req);

	int data_len = evbuffer_get_length(evbuf);
	char *data = (char *)evbuffer_pullup(evbuf, data_len);

	if (data_len <= 0) return;
	if (data == NULL) return;
		
	size_t copy_len = data_len > POST_BUF_MAX ? POST_BUF_MAX : data_len;
	memcpy(buff, data, copy_len);

	evbuffer_drain(evbuf, data_len);
	
	{

		struct evkeyvalq params;
		int ret = evhttp_parse_query_str(buff, &params);

		const char* _a = evhttp_find_header(&params, "a");
		const char* _b = evhttp_find_header(&params, "b");
		const char* _c = evhttp_find_header(&params, "c");
		const char* _d = evhttp_find_header(&params, "d");

	}

	
	{
		const char* host = evhttp_request_get_host(req);

		rapidjson::Document doc;
		doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		doc.AddMember("host", host, allocator);
		KVPragma kv;
		kv.parse_url(buff);
		kv.printf([&doc, &allocator](KV* pkv)
		{
			const char* k = pkv->k;
			const char* v = pkv->v;

			doc.AddMember(k, v, allocator);
		});

		rapidjson::StringBuffer buffer;
		rapidjson::Writer< rapidjson::StringBuffer > writer(buffer);
		doc.Accept(writer);

		const char* str = buffer.GetString();
		lw_http_send_reply(req, str);
	}

	return;
}

static void get_add_cb(struct evhttp_request *req, void *arg)
{
	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) 
	{
		lw_http_send_reply(req, "{\"code\":0,\"what\":\"The requested resource does not support http method 'POST'.""}");
		return;
	}

// 	{
// 		char buff[POST_BUF_MAX] = "\0";
// 
// 		struct evbuffer *evbuf;
// 		evbuf = evhttp_request_get_input_buffer(req);
// 
// 		int data_len = evbuffer_get_length(evbuf);
// 		char *data = (char *)evbuffer_pullup(evbuf, data_len);
// 
// 		if (data_len <= 0) return;
// 		if (data == NULL) return;
// 
// 		size_t copy_len = data_len > POST_BUF_MAX ? POST_BUF_MAX : data_len;
// 		memcpy(buff, data, copy_len);
// 
// 		evbuffer_drain(evbuf, data_len);
// 
// 		KVPragma kv;
// 		kv.parse_url(buff);
// 		const char* _a = kv.find_value("a");
// 		const char* _b = kv.find_value("b");
// 		const char* _c = kv.find_value("c");
// 		const char* _d = kv.find_value("d");
// 	}

	struct evkeyvalq http_query;
	do
	{
		const char *uri = evhttp_request_get_uri(req);
		int ret = evhttp_parse_query_str(uri, &http_query);
		if (ret != 0)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		//解析URI的参数(即GET方法的参数)
		struct evkeyvalq params;
		ret = evhttp_parse_query(uri, &params);
		if (ret != 0)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		const char* _a = evhttp_find_header(&params, "a");
		const char* _b = evhttp_find_header(&params, "b");

		if (_a == NULL || _b == NULL)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		int a = std::atoi(_a);
		int b = std::atoi(_b);
		
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "%d + %d = %d", a, b, a + b);
		evhttp_send_reply(req, HTTP_OK, NULL, buf);
		evbuffer_free(buf);

	} while (0);

	evhttp_clear_headers(&http_query);

	return;
}


static void get_sub_cb(struct evhttp_request *req, void *arg)
{
	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET)
	{
		lw_http_send_reply(req, "{\"code\":0,\"what\":\"please use get method request!\"}");
		return;
	}

	struct evkeyvalq http_query;
	do
	{
		const char *uri = evhttp_request_get_uri(req);
		int ret = evhttp_parse_query_str(uri, &http_query);
		if (ret != 0)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		//解析URI的参数(即GET方法的参数)
		struct evkeyvalq params;
		ret = evhttp_parse_query(uri, &params);
		if (ret != 0)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		const char* _a = evhttp_find_header(&params, "a");
		const char* _b = evhttp_find_header(&params, "b");

		if (_a == NULL || _b == NULL)
		{
			lw_http_send_reply(req, "paragma is error!");
			break;
		}

		int a = std::atoi(_a);
		int b = std::atoi(_b);
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "%d - %d = %d", a, b, a - b);

		evhttp_send_reply(req, HTTP_OK, NULL, buf);

		evbuffer_free(buf);

	} while (0);

	evhttp_clear_headers(&http_query);

	return;
}

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

void __run_http_server(unsigned short port)
{
	char uri_root[512];

	const char *addr = "127.0.0.1";
	struct evhttp *httpServ = NULL;
	struct evhttp_bound_socket *handle = NULL;

	struct event_config *cfg = event_config_new();
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg)
	{
		__http_base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}

	httpServ = evhttp_new(__http_base);
	if (!httpServ)
	{
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");

		event_base_free(__http_base);

		return;
	}

	handle = evhttp_bind_socket_with_handle(httpServ, addr, port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. exiting.\n", (int)port);
		
		evhttp_free(httpServ);

		return;
	}

	// 设置超时
	evhttp_set_timeout(httpServ, 120);

	// 设置回调  
	evhttp_set_gencb(httpServ, http_default_handler, NULL);

	////设置路由 post method
	//{
	//	evhttp_set_cb(httpServ, "/login", post_login_cb, httpServ);
	//}

	////设置路由 get method
	//{
	//	evhttp_set_cb(httpServ, "/add", get_add_cb, httpServ);
	//}	

	for (size_t i = 0; i < sizeof(httpgignature)/sizeof(httpgignature[0]); i++)
	{
		evhttp_set_cb(httpServ, httpgignature[i]._signature, httpgignature[i]._cb, httpServ);
	}

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
				printf("本地HTTP服务启动完成 [http://%s:%d]\n", addr, got_port);
				evutil_snprintf(uri_root, sizeof(uri_root), "http://%s:%d", addr, got_port);
			}
			else
			{
				fprintf(stderr, "evutil_inet_ntop failed\n");
				break;
			}

		} while (0);
		
	}

	int ret = event_base_dispatch(__http_base);

	evhttp_free(httpServ);

	event_base_free(__http_base);
}

int run_http_server(unsigned short port)
{
	//自定义信号处理函数
// 	signal(SIGINT, signal_handler);
// 	signal(SIGILL, signal_handler);
// 	signal(SIGFPE, signal_handler);
// 	signal(SIGSEGV, signal_handler);
// 	signal(SIGTERM, signal_handler);
// 	signal(SIGBREAK, signal_handler);
// 	signal(SIGABRT, signal_handler);

	std::thread t(__run_http_server, port);
	t.detach();

	return 0;
}