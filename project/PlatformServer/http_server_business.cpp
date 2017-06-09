#include "http_server_business.h"

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

#include "http_server.h"

HttpServer __g_http_serv;

#define POST_BUF_MAX			1024*4

static void http_default_handler(struct evhttp_request *req, void *arg);
static const char* lw_get_command(struct evhttp_request * req);
static void lw_http_send_reply(struct evhttp_request * req, const char* what);
static void login_cb(struct evhttp_request *req, void *arg);
static void add_cb(struct evhttp_request *req, void *arg);
static void sub_cb(struct evhttp_request *req, void *arg);

HTTP_BUSINESS_SIGNATURE __g_httpsignature[] =
{
	HTTP_BUSINESS_SIGNATURE("/login", "POST", login_cb),
	HTTP_BUSINESS_SIGNATURE("/add", "GET", add_cb),
	HTTP_BUSINESS_SIGNATURE("/sub", "GET", sub_cb),
};

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

static void http_default_handler(struct evhttp_request *req, void *arg)
{
	evhttp_send_error(req, HTTP_BADREQUEST, "sorry，你走错地方了！");
	return;
}

static void login_cb(struct evhttp_request *req, void *arg)
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

static void add_cb(struct evhttp_request *req, void *arg)
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


static void sub_cb(struct evhttp_request *req, void *arg)
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

void __init_http_business(lw_int32 port)
{
	// 设置回调函数
	__g_http_serv.start_listener("172.16.1.61", port, [](HttpServer* server) -> lw_int32
	{
		// 设置标准接口回调函数
		server->set_gen_cb(http_default_handler, NULL);

		for (size_t i = 0; i < sizeof(__g_httpsignature) / sizeof(__g_httpsignature[0]); i++)
		{
			server->set_cb(__g_httpsignature[i]._signature, __g_httpsignature[i]._cb, server);
		}
		return 0;
	});
}