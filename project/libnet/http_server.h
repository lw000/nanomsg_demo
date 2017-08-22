#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <functional>
#include <string>
#include <unordered_map>

#include "common_type.h"

class HttpServer;
struct event_base;
struct evhttp;
struct evhttp_request;

typedef void(*HTTP_CB)(struct evhttp_request *);


struct HTTP_METHOD_SIGNATURE
{
	std::string _signature;
	lw_uint32 _cmd;
	HTTP_CB _cb;
};

void lw_http_send_reply(struct evhttp_request * req, const char* what);

class HttpServer final
{
	typedef std::unordered_map<std::string, HTTP_METHOD_SIGNATURE*> MAP_METHOD;
	typedef MAP_METHOD::iterator iterator;
	typedef MAP_METHOD::const_iterator const_iterator;

public:
	HttpServer();
	~HttpServer();

public:
	lw_int32 create(const char* addr, lw_uint32 port);
	void run();
	void gen(HTTP_CB cb);
	void get(const char * path, HTTP_CB cb);
	void post(const char * path, HTTP_CB cb);

private:
	void __run();

private:
	struct event_base* _base;
	struct evhttp *_htpServ;

private:
	std::string _addr;
	lw_uint32 port;
	std::unordered_map<std::string, HTTP_METHOD_SIGNATURE*> _method;
};

#endif // !__HTTP_SERVER_H__