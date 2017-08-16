#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <functional>
#include <string>
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

#include "common_type.h"

typedef void(*LW_HTTP_CB)(struct evhttp_request *);

class HttpServer;

struct HTTP_METHOD_SIGNATURE
{
	std::string _signature;
	lw_uint32 _cmd;
	LW_HTTP_CB _cb;
};

void lw_http_send_reply(struct evhttp_request * req, const char* what);

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

public:
	lw_int32 init(const char* addr, lw_uint32 port);
	void start();
	void set_http_gen_hook(LW_HTTP_CB cb);
	void get(const char * path, LW_HTTP_CB cb);
	void post(const char * path, LW_HTTP_CB cb);

private:
	void __run();

private:
	struct event_base* _base;
	struct evhttp *_httpServ;
	std::string _addr;
	lw_uint32 port;
	std::unordered_map<std::string, HTTP_METHOD_SIGNATURE*> _unmap_method;
};

#endif // !__HTTP_SERVER_H__