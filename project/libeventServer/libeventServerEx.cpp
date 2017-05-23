// libevent-demo.cpp : 定义控制台应用程序的入口点。
//

#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/thread.h>

#include "base_type.h"
#include "common_marco.h"
#include "NetMessage.h"
#include "business.h"

#include "Message.h"
#include "platform.pb.h"

using namespace LW;

static void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int, void *);
static void read_cb(struct bufferevent *, void *);
static void write_cb(struct bufferevent *, void *);
static void event_cb(struct bufferevent *, short, void *);
static void signal_cb(evutil_socket_t, short, void *);


static lw_int32 send_socket_data(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize);
static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	struct bufferevent *bev = (struct bufferevent *)userdata;
	switch (cmd)
	{
	case 100:
	{
		{
			platform::csc_msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf(" order: %d\n", msg.order());
		}
		
		{
			platform::csc_msg_heartbeat msg;
			msg.set_order(100);

			int len = (int)msg.ByteSizeLong();
			char s[256] = { 0 };
			bool ret = msg.SerializeToArray(s, len);
			if (ret) {
				send_socket_data(bev, 100, s, len);
			}
		}
	} break;
	case 10001:
	{
		platform::sc_msg_request_userinfo client_userinfo;
		client_userinfo.ParseFromArray(buf, bufsize);
		printf(" userid: %d\n", client_userinfo.userid());

		platform::sc_msg_userinfo userinfo;
		userinfo.set_userid(client_userinfo.userid());
		userinfo.set_age(30);
		userinfo.set_sex(1);
		userinfo.set_name("liwei");
		userinfo.set_address("guangdong shenzhen");

		char s[256] = { 0 };
		bool ret = userinfo.SerializePartialToArray(s, sizeof(s));

		send_socket_data(bev, 20001, s, strlen(s));

	} break;
	default:
		break;
	}
}

static lw_int32 send_socket_data(struct bufferevent *bev, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);

	lw_int32 result = bufferevent_write(bev, p->buf, p->size);
	lw_free_net_message(p);

	return result;
}

static void signal_cb(evutil_socket_t fd, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = { 1, 0 };

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);
}

static void write_cb(struct bufferevent *bev, void *user_data)
{
	struct evbuffer *output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0)
	{
		//printf("flushed answer \n");


		//bufferevent_free(bev);
	}
}

static void read_cb(struct bufferevent *bev, void *user_data)
{
	// 从客户端读取数据
	{
		struct evbuffer *input;
		input = bufferevent_get_input(bev);
		size_t input_len = evbuffer_get_length(input);

		char *read_buf = (char*)malloc(input_len);
		size_t read_len = bufferevent_read(bev, read_buf, input_len);

		if (read_len == input_len)
		{
			if (lw_on_parse_socket_data(read_buf, read_len, on_socket_recv, bev) == 0)
			{

			}
		}	

		free(read_buf);
	}
}

static void event_cb(struct bufferevent *bev, short event, void *user_data)
{
	if (event & BEV_EVENT_EOF)
	{
		printf("Connection closed.\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		printf("Got an error on the connection: %s\n", strerror(errno));/*XXX win32*/
	}

	/* None of the other events can happen here, since we haven't enabled
	* timeouts */
	bufferevent_free(bev);
}

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;

	struct bufferevent *bev;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		fprintf(stderr, "error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}

	bufferevent_setcb(bev, read_cb, write_cb, event_cb, nullptr);
	bufferevent_enable(bev, EV_WRITE | EV_READ);
//	bufferevent_enable(bev, EV_READ);

	{
		platform::csc_msg_heartbeat msg;
		msg.set_order(2);
		int len = (int)msg.ByteSizeLong();
		char buf[256] = { 0 };
		bool ret = msg.SerializePartialToArray(buf, 256);
		send_socket_data(bev, 100, buf, len);
	}
}

static void run_server(u_short port = 9876)
{
	struct event_base *base;
	base = event_base_new();
	if (!base)
	{
		return;
	}

	struct sockaddr_in sin;
 	sin.sin_family = AF_INET;
 	sin.sin_addr.s_addr = 0;
 	sin.sin_port = htons(port);

	struct evconnlistener *listener;
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE, -1, (struct sockaddr*)&sin, sizeof(sin));

	const char* method = event_base_get_method(base);

	struct event *signal_event;
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
	if (!signal_event || event_add(signal_event, nullptr) < 0)
	{
		fprintf(stderr, "Could not create/add a signal event!\n");
		return;
	}

	event_base_dispatch(base);

	event_free(signal_event);
	evconnlistener_free(listener);
	event_base_free(base);
}

void log_cb(int severity, const char *msg)
{

}

int main(int argc, char** argv)
{
#if defined(WIN32) || defined(_WIN32)
	WSADATA WSAData;
	int ret;
	if (ret = WSAStartup(MAKEWORD(2, 2), &WSAData))
	{
		std::cout << "can not initilize winsock.dll" << std::endl;
		std::cout << "error code:" << WSAGetLastError() << std::endl;
		return 1;
	}
#endif

	event_set_log_callback(log_cb);

	evthread_use_windows_threads();

	std::thread t(run_server, 9876);
	t.join();

#if defined(WIN32) || defined(_WIN32)
	WSACleanup();
#endif

	return 0;
}

