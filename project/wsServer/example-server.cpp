#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <libwebsockets.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include "example.h"

static int destroy_flag = 0;
static struct event_base *event_server_loop;

struct session_data {
	int fd;
};

static void __signal_cb(int signo)
{
	destroy_flag = 1;
	event_base_loopbreak(event_server_loop);
	printf("sign no : %d\n", signo);
}

BOOL CALLBACK __cosonle_handler(DWORD ev)
{
// #define CTRL_C_EVENT        0
// #define CTRL_BREAK_EVENT    1
// #define CTRL_CLOSE_EVENT    2
// 	// 3 is reserved!
// 	// 4 is reserved!
// #define CTRL_LOGOFF_EVENT   5
// #define CTRL_SHUTDOWN_EVENT 6

	BOOL bRet = FALSE;
	switch (ev)
	{
	case CTRL_C_EVENT:
	{
		destroy_flag = 1;
		event_base_loopbreak(event_server_loop);
	} break;
	case CTRL_BREAK_EVENT:
	{
		destroy_flag = 1;
		event_base_loopbreak(event_server_loop);
	} break;
	case CTRL_CLOSE_EVENT:
		destroy_flag = 1;
		event_base_loopbreak(event_server_loop);
		bRet = TRUE;
		break;
	default:
		break;
	}
	printf("sign no : %d\n", ev);
	return bRet;
}

/* *
* websocket_write_back: write the string data to the destination wsi.
*/
int websocket_write(struct lws *wsi_in, char *str, int str_size_in)
{
	if (str == NULL || wsi_in == NULL)
		return -1;

	int n;
	int len;
	char *out = NULL;

	len = (str_size_in < 1) ? strlen(str) : str_size_in;

	out = (char *)malloc(sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
	//* setup the buffer*/
	memcpy(out + LWS_SEND_BUFFER_PRE_PADDING, str, len);
	//* write out*/
	n = lws_write(wsi_in, (unsigned char*)out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	printf("[websocket_write_back] %s\n", str);
	//* free the buffer*/
	free(out);

	return n;
}

static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) 
	{
	case LWS_CALLBACK_ESTABLISHED:
		printf("[Main Service] Connection established\n");
		break;

		//* If receive a data from client*/
	case LWS_CALLBACK_RECEIVE:
		printf("[Main Service] Server recvived:%s\n", (char *)in);

		//* echo back to client*/
		websocket_write(wsi, (char *)in, -1);

		break;
	case LWS_CALLBACK_CLOSED:
		printf("[Main Service] Client close.\n");
		break;

	case LWS_CALLBACK_HTTP: {
		char buf[256];
		int n;
		printf("lws_http_serve: %s\n", (const char *)in);
		n = 0;
		while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf),
			WSI_TOKEN_HTTP_URI_ARGS, n) > 0) {
			lwsl_notice("URI Arg %d: %s\n", ++n, buf);
		}

		{
			lws_get_peer_simple(wsi, buf, sizeof(buf));
			lwsl_info("HTTP connect from %s\n", buf);
		}

		if (len < 1) {
			lws_return_http_status(wsi, HTTP_STATUS_BAD_REQUEST, NULL);
		}
		else
		{
			lws_return_http_status(wsi, HTTP_STATUS_OK, "{\"resply\":\"0\"}");
		}
		
	} break;
	case LWS_CALLBACK_CLOSED_HTTP:
	{

	} break;
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] =
{
	{
		"my-protocol",
		ws_service_callback,
		sizeof(struct session_data),
		0,
		0,
		0,
		0
	},
	{ NULL, NULL, 0, 0, 0, 0, 0 }	 /* End of list */ 
};

static const struct lws_extension exts[] = {
	{ NULL, NULL, NULL /* terminator */ }
};

int test_server(int argc, char **argv)
{
#ifdef _WIN32
	SetConsoleCtrlHandler(__cosonle_handler, TRUE);
#else
	//* register the signal SIGINT handler */
	struct sigaction act;
	act.sa_handler = __signal_cb;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, 0);
#endif

	// server url will usd port 5000
	int port = 5000;
	struct lws_context_creation_info info;
	struct lws_context *context;

	//* setup websocket context info*/
	memset(&info, 0, sizeof info);
	info.port = port;
	info.iface = NULL;
	info.protocols = protocols;
	info.extensions = NULL;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;
	info.gid = -1;
	info.uid = -1;
	info.options = LWS_SERVER_OPTION_LIBEVENT;
	//info.options = 0;

	//* create libwebsocket context. */
	context = lws_create_context(&info);
	if (context == NULL)
	{
		printf("[Main] Websocket context create error.\n");
		return -1;
	}

	printf("[Main] Websocket context create success.\n");

	event_server_loop = event_base_new();

	{
		// Don't use the default Signal Event Watcher & Handler
		lws_event_sigint_cfg(context, 0, NULL);
		// Initialize the LWS with libevent loop
		lws_event_initloop(context, event_server_loop, 0);

		event_base_dispatch(event_server_loop);
	}

	//* websocket service */
	while (!destroy_flag)
	{
		lws_service(context, 50);
	}

#ifdef _WIN32
	Sleep(1000);
#else
	usleep(10);
#endif	// !_WIN32

	lws_context_destroy(context);

	return 0;
}