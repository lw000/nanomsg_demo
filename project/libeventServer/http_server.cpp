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
//#include <event2/event_compat.h>
#include <event2/buffer.h>
//#include <event2/buffer_compat.h>
#include <event2/http.h>
//#include <event2/http_compat.h>
#include <event2/http_struct.h>
#include <event2/util.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"

#define POST_BUF_MAX			1024*4

static struct event_base* __http_base = NULL;

static char * lw_strtok_r(char *s, const char *delim, char **state) {
	char *cp, *start;
	start = cp = s ? s : *state;
	if (!cp)
		return NULL;
	while (*cp && !strchr(delim, *cp))
		++cp;
	if (!*cp) {
		if (cp == start)
			return NULL;
		*state = NULL;
		return start;
	}
	else {
		*cp++ = '\0';
		*state = cp;
		return start;
	}
}

static std::vector<std::string> split(const char* str, const char* pattern)
{
	char *p = NULL;
	char *p1 = NULL;
	p = lw_strtok_r(const_cast<char*>(str), pattern, &p1);
	std::vector<std::string> s;
	while (p != NULL)
	{
		s.push_back(p);
		p = lw_strtok_r(NULL, pattern, &p1);
	}
	return s;
}

static std::unordered_map<std::string, std::string> split_url_pragma_data(const char* str)
{
	std::unordered_map<std::string, std::string> s;

	char *p = NULL;
	char *p1 = NULL;
	p = lw_strtok_r(const_cast<char*>(str), "&", &p1);
	while (p != NULL)
	{
		{
			char *q = NULL;
			char *q1 = NULL;
			std::string q2;
			std::string q3;

			q = lw_strtok_r(const_cast<char*>(p), "=", &q1);
			q2 = q;
			q = lw_strtok_r(NULL, "=", &q1);
			q3 = q;
			s[q2] = q3;
		}
		p = lw_strtok_r(NULL, "&", &p1);
	}
	return s;
}

static void lw_http_reply(struct evhttp_request * req, const char* what)
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
		lw_http_reply(req, "0");
		return;
	}


	char buff[POST_BUF_MAX] = "\0";

	int data_len = evbuffer_get_length(req->input_buffer);
	char *data = (char *)evbuffer_pullup(req->input_buffer, -1);

	std::string out;

	if (data_len > 0)
	{
		size_t copy_len = data_len > POST_BUF_MAX ? POST_BUF_MAX : data_len;
		memcpy(buff, data, copy_len);
		out.assign(buff, copy_len);
	}

	std::unordered_map<std::string, std::string> urldata = split_url_pragma_data(buff);

	{
		const char* host = evhttp_request_get_host(req);

		rapidjson::Document doc;
		doc.SetObject();
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		doc.AddMember("host", host, allocator);
		doc.AddMember("length", out.size(), allocator);
		
		for (auto d : urldata)
		{
			const char* k = d.first.c_str();
			const char* v = d.second.c_str();

			doc.AddMember(k, v, allocator);
		}

		rapidjson::StringBuffer buffer;
		rapidjson::Writer< rapidjson::StringBuffer > writer(buffer);
		doc.Accept(writer);

		const char* str = buffer.GetString();
		lw_http_reply(req, str);
	}

	return;
}

static void get_add_cb(struct evhttp_request *req, void *arg)
{
	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) 
	{
		lw_http_reply(req, "0");
		return;
	}

	struct evkeyvalq http_query;
	do
	{
		const char *uri = evhttp_request_get_uri(req);
		int ret = evhttp_parse_query_str(uri, &http_query);
		if (ret != 0)
		{
			lw_http_reply(req, "paragma is error!");
			break;
		}

		//解析URI的参数(即GET方法的参数)
		struct evkeyvalq params;
		ret = evhttp_parse_query(uri, &params);
		if (ret != 0)
		{
			lw_http_reply(req, "paragma is error!");
			break;
		}

		const char* _a = evhttp_find_header(&params, "a");
		const char* _b = evhttp_find_header(&params, "b");

		if (_a == NULL)
		{
			lw_http_reply(req, "a paragma is error!");
			break;
		}

		if (_b == NULL)
		{
			lw_http_reply(req, "b paragma is error!");
			break;
		}

		int a = std::atoi(_a);
		int b = std::atoi(_b);
		struct evbuffer *buf = evbuffer_new();
		evbuffer_add_printf(buf, "%d + %d = %d", a, b, a + b);
		
		evhttp_send_reply(req, HTTP_OK, "1", buf);

		evbuffer_free(buf);

		printf("%d + %d = %d\n", a, b, a + b);

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

	//设置路由 post method
	{
		evhttp_set_cb(httpServ, "/login", post_login_cb, httpServ);
	}

	//设置路由 get method
	{
		evhttp_set_cb(httpServ, "/add", get_add_cb, httpServ);
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