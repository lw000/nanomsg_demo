#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <functional>
#include <string>
#include <unordered_map>
#include "common_type.h"

struct event_base;
struct evhttp;
struct evhttp_request;
struct HTTP_METHOD_SIGNATURE;
class HttpServer;

void lw_http_send_reply(struct evhttp_request * req, const char* what);

class HttpServer final
{
	friend class CoreHttp;

	typedef std::unordered_map<std::string, HTTP_METHOD_SIGNATURE*> MAP_METHOD;
	typedef MAP_METHOD::iterator iterator;
	typedef MAP_METHOD::const_iterator const_iterator;

public:
	HttpServer();
	~HttpServer();

public:
	lw_int32 create(const char* addr, lw_uint32 port);
	void run();
	void gen(std::function<void (struct evhttp_request *)> cb);
	void get(const char * path, std::function<void(struct evhttp_request *)> cb);
	void post(const char * path, std::function<void(struct evhttp_request *)> cb);

private:
	void __run();
	void __doStore(const char * path, lw_int32 cmd, std::function<void(struct evhttp_request *)> cb);

private:
	struct event_base* _base;
	struct evhttp *_htpServ;

private:
	std::string _addr;
	lw_uint32 port;
	MAP_METHOD _method;
	std::function<void(struct evhttp_request *)> _gen_cb;
};

#endif // !__HTTP_SERVER_H__