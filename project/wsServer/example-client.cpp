#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <signal.h>
#include <thread>

#include <libwebsockets.h>

#include "example.h"
#include <unordered_map>
#include <mutex>
#include <string>

// static int destroy_flag = 0;
// static int connection_flag = 0;

std::mutex _g_mutex;

struct session_data {
	int fd;
};

struct pthread_args {
	struct lws_context *context;
	struct lws *ws;
	std::string name;
	int destroy_flag;
	int connection_flag;
};

static std::unordered_map <lws*, pthread_args*> __g_pthread_args;

static void __signal_cb(int signo);
static int sendMessageText(struct lws *wsi_in, char *str, int str_size_in);
static int sendMessagePing(struct lws *wsi_in);
static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user,
	void *in, size_t len);

static struct lws_protocols protocols[] =
{
	{
		"my-websocket-protocol",
		ws_service_callback,
		sizeof(struct session_data),
		0,
		0,
		NULL,
		0
	},
	{
		NULL, NULL, 0, 0, 0, NULL, 0   /* End of list */
	}
};


static void __signal_cb(int signo) {

	for (auto v : __g_pthread_args)
	{
		v.second->destroy_flag = 1;
	}
}

static int sendMessageText(struct lws *wsi_in, char *str, int str_size_in) {
	if (str == NULL || wsi_in == NULL) {
		return -1;
	}
		
	int n;
	int len;
	char *out = NULL;

	len = (str_size_in < 1) ? strlen(str) : str_size_in;

	out = (char *)malloc(sizeof(char)*(LWS_PRE + len));
	//* setup the buffer*/
	memcpy(out + LWS_PRE, str, len);
	//* write out*/
	n = lws_write(wsi_in, (unsigned char*)out + LWS_PRE, len, LWS_WRITE_TEXT);

	//* free the buffer*/
	free(out);

	return n;
}

static int sendMessagePing(struct lws *wsi) {

	unsigned char pingbuf[LWS_PRE + 1];
	int n = lws_write(wsi, pingbuf + LWS_PRE, 1, LWS_WRITE_PING);
	return n;
}

static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED: {
		struct pthread_args *tool = (struct pthread_args *)__g_pthread_args[wsi];
		printf("Connect[%s] with server success.\n", tool->name.c_str());
		tool->connection_flag = 1;
	} break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
		struct pthread_args *tool = (struct pthread_args *)__g_pthread_args[wsi];
		printf("Connect[%s] with server error.\n", tool->name.c_str());
		tool->destroy_flag = 1;
		tool->connection_flag = 0;
	} break;
	case LWS_CALLBACK_CLOSED: {	
		struct pthread_args *tool = (struct pthread_args *)__g_pthread_args[wsi];
		printf("Connect[%s] LWS_CALLBACK_CLOSED\n", tool->name.c_str());
		tool->destroy_flag = 1;
		tool->connection_flag = 0;
	} break;
	case LWS_CALLBACK_CLIENT_RECEIVE: {
		struct pthread_args *tool = (struct pthread_args *)__g_pthread_args[wsi];
		char* buf = (char*)malloc(len + 1);
		memcpy(buf, (char*)in, len);
		buf[len] = '\0';
		printf("Client[%s] recvived:%s\n", tool->name.c_str(), buf);
		free(buf);
	} break;
	case LWS_CALLBACK_CLIENT_WRITEABLE: {
		sendMessagePing(wsi);
	} break;
	default:
		break;
	}

	return 0;
}

static void *pthread_routine(void *tool_in)
{
	struct pthread_args *tool = (struct pthread_args *)tool_in;

	//* waiting for connection with server done.*/
	while (!tool->connection_flag)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	while (!tool->destroy_flag)
	{
		static int i = 0;
		char buf[128];
		sprintf(buf, "{\"data\":\"good [%d]\"}", i++);
		sendMessageText(tool->ws, buf, strlen(buf));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	return nullptr;
}

static void create_ws_client(int v)
{
	struct lws_context *context = NULL;
	struct lws_context_creation_info creation_info;
	struct lws_client_connect_info connect_info;
	struct lws *wsi = NULL;

	memset(&creation_info, 0, sizeof creation_info);
	creation_info.port = CONTEXT_PORT_NO_LISTEN;
	creation_info.iface = NULL;
	creation_info.protocols = protocols;
	creation_info.ssl_cert_filepath = NULL;
	creation_info.ssl_private_key_filepath = NULL;
	creation_info.extensions = NULL;
	creation_info.gid = -1;
	creation_info.uid = -1;
	creation_info.options = 0;

	context = lws_create_context(&creation_info);
	if (context == NULL)
	{
		lwsl_notice("[Main] context is NULL.\n");
		return;
	}
	lwsl_notice("[Main] context created.\n");
	
	memset(&connect_info, 0, sizeof(connect_info));
	connect_info.context = context;
	connect_info.address = "localhost";
	connect_info.port = 5000;
	connect_info.ssl_connection = 0;
	char buf[512];
	sprintf(buf, "/ws?rid=%d&uid=%d&extra=abcdedfhigklmnopqrstuvwxyz%d", 1, v + 10000, v + 10000);
	connect_info.path = buf;
	connect_info.host = "localhost:5000";
	connect_info.origin = "localhost:5000";
	connect_info.protocol = "my-websocket-protocol";
	connect_info.ietf_version_or_minus_one = -1;
	
	wsi = lws_client_connect_via_info(&connect_info);
	if (wsi == NULL) {
		lwsl_err("[Main] wsi create error.\n");
		return;
	}

	lwsl_notice("[Main] wsi create success.\n");

	struct pthread_args* tool = new struct pthread_args;
	tool->ws = wsi;
	tool->context = context;
	tool->connection_flag = 0;
	tool->destroy_flag = 0;
	tool->name = std::to_string(v);

	{
		std::lock_guard<std::mutex> l(_g_mutex);
		__g_pthread_args.insert(std::pair<lws*, pthread_args*>(wsi, tool));
	}
	
	{
		std::thread a(pthread_routine, tool);
		a.detach();
	}

	while (!tool->destroy_flag)
	{
		lws_service(context, 50);
	}

	lws_context_destroy(context);
}

int test_ws_client(int argc, char **argv)
{
	for (int i = 0; i < 10; i++)
	{
		std::thread a(create_ws_client, i);
		a.detach();
	}

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}