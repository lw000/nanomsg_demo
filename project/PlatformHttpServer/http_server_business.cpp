#include "http_server_business.h"

#include "http_server.h"

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

#include <stdio.h>
#include <signal.h>
#include <string>
#include <vector>

HttpServer __g_htpServ;

#define POST_BUF_MAX			1024*4

static void default_cb(struct evhttp_request *req);
static void login_post_cb(struct evhttp_request *req);
static void add_get_cb(struct evhttp_request *req);
static void sub_get_cb(struct evhttp_request *req);

static void default_cb(struct evhttp_request *req)
{
	evhttp_send_error(req, HTTP_BADREQUEST, "sorry，你走错地方了！");
	return;
}

static void login_post_cb(struct evhttp_request *req)
{
	char buff[POST_BUF_MAX] = "\0";
	{
		struct evbuffer *evbuf;
		evbuf = evhttp_request_get_input_buffer(req);

		int data_len = evbuffer_get_length(evbuf);
		char *data = (char *)evbuffer_pullup(evbuf, data_len);

		if (data_len <= 0) return;
		if (data == NULL) return;

		size_t copy_len = data_len > POST_BUF_MAX ? POST_BUF_MAX : data_len;
		memcpy(buff, data, copy_len);

		evbuffer_drain(evbuf, data_len);
	}

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
		kv.parseURL(buff);
		kv.each([&doc, &allocator](KV* pkv)
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

static void add_get_cb(struct evhttp_request *req)
{

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

static void sub_get_cb(struct evhttp_request *req)
{
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


static void mul_get_cb(struct evhttp_request *req)
{

}

void __create_http_service_business(lw_int32 port)
{
	// 设置回调函数
	int ret = __g_htpServ.create("127.0.0.1", port);
	if (ret == 0)
	{
		// 设置标准接口回调函数
		__g_htpServ.gen(default_cb);

		__g_htpServ.post("/login", login_post_cb);
		__g_htpServ.get("/add", add_get_cb);
		__g_htpServ.get("/sub", sub_get_cb);
		__g_htpServ.get("/mul", mul_get_cb);

		__g_htpServ.run();
	}
}