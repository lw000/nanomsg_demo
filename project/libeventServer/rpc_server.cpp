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
#endif

#include <thread>

#include <event2/event.h>
#include <event2/event-config.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/buffer.h>
#include <event2/buffer_compat.h>
#include <event2/http.h>
#include <event2/http_compat.h>
#include <event2/http_struct.h>
#include <event2/rpc.h>
#include <event2/rpc_compat.h>
#include <event2/rpc_struct.h>
#include <event2/util.h>

static void * _req_new_handler(void *arg)
{
	return;
}

static void _req_free_handler(void *arg)
{
	return;
}

static int _req_unmarshal_handler(void *, struct evbuffer *)
{
	return 0;
}

static void * _rpl_new_handler(void *arg)
{
	return;
}

static void _rpl_free_handler(void *arg)
{
	return;
}

static int _rpl_complete_handler(void *)
{
	return 0;
}

static void _rpl_unmarshal_handler(struct evbuffer *, void *)
{
	return;
}

static void _add(struct evrpc_req_generic *, void *)
{

}


void signal_handler(int sig) {
	switch (sig) {
	case SIGINT:
	case SIGILL:
	case SIGFPE:
	case SIGSEGV:
	case SIGTERM:
	case SIGBREAK:
	case SIGABRT:
		event_loopbreak();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
		break;
	}
}

void run_rpc_server(unsigned short port)
{
	char uri_root[512];

	const char *addr = "127.0.0.1";

	struct evhttp *httpServ = NULL;
	struct evrpc_base *rpcServ = NULL;
	struct evhttp_bound_socket *handle = NULL;

	struct event_base* base = NULL;
	struct event_config *cfg = event_config_new();
	event_config_set_flag(cfg, EVENT_BASE_FLAG_STARTUP_IOCP);
	if (cfg)
	{
		base = event_base_new_with_config(cfg);
		event_config_free(cfg);
	}

	httpServ = evhttp_new(base);
	if (!httpServ)
	{
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");

		event_base_free(base);

		return;
	}

	rpcServ = evrpc_init(httpServ);
	if (!httpServ)
	{
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");

		event_base_free(base);

		return;
	}

	handle = evhttp_bind_socket_with_handle(httpServ, addr, port);
	if (!handle) {
		fprintf(stderr, "couldn't bind to port %d. exiting.\n", (int)port);
		
		evhttp_free(httpServ);

		return;
	}
	
// 	evrpc_register_generic(struct evrpc_base *base, const char *name,
// 		void(*callback)(struct evrpc_req_generic *, void *), void *cbarg,
// 		void *(*req_new)(void *), void *req_new_arg, void(*req_free)(void *),
// 		int(*req_unmarshal)(void *, struct evbuffer *),
// 		void *(*rpl_new)(void *), void *rpl_new_arg, void(*rpl_free)(void *),
// 		int(*rpl_complete)(void *),
// 		void(*rpl_marshal)(struct evbuffer *, void *));

	evrpc_register_generic(rpcServ, "liwei", 
		_add, NULL, 
		_req_new_handler, NULL, _req_free_handler,
		_req_unmarshal_handler,
		_rpl_new_handler, NULL, _rpl_free_handler,
		_rpl_complete_handler,
		_rpl_unmarshal_handler);

	int ret = event_base_dispatch(base);

	evrpc_unregister_rpc(rpcServ, "liwei");

	evrpc_free(rpcServ);

	evhttp_free(httpServ);

	event_base_free(base);
}

int rpc_server_run(int argc, char **argv)
{
	//自定义信号处理函数
// 	signal(SIGINT, signal_handler);
// 	signal(SIGILL, signal_handler);
// 	signal(SIGFPE, signal_handler);
// 	signal(SIGSEGV, signal_handler);
// 	signal(SIGTERM, signal_handler);
// 	signal(SIGBREAK, signal_handler);
// 	signal(SIGABRT, signal_handler);

	std::thread t(run_rpc_server, 9878);
	t.detach();

	return 0;
}