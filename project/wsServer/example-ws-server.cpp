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

struct per_session_data__http {
	lws_fop_fd_t fop_fd;
};

static void __signal_cb(int signo)
{
	destroy_flag = 1;
	event_base_loopbreak(event_server_loop);
	lwsl_notice("sign no : %d\n", signo);
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
	lwsl_debug("sign no : %d\n", ev);
	return bRet;
}

/* *
* websocket_write_back: write the string data to the destination wsi.
*/
int sendMessageText(struct lws *ws, char *str, int str_size_in)
{
	if (str == NULL || ws == NULL) {
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
	n = lws_write(ws, (unsigned char*)out + LWS_PRE, len, LWS_WRITE_TEXT);

	//* free the buffer*/
	free(out);

	return n;
}

static int sendMessagePingFrame(struct lws *wsi_in) {
	unsigned char pingbuf[LWS_PRE + 1];
	int n = lws_write(wsi_in, pingbuf + LWS_PRE, 1, LWS_WRITE_PING);
	return n;
}

static int ws_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	lwsl_notice("lws_callback_reasons: %d \n", reason);
	switch (reason) {
	case LWS_CALLBACK_ESTABLISHED: {
		lwsl_notice("[Main Service] Connection established\n");
	} break;
		//* If receive a data from client*/
	case LWS_CALLBACK_RECEIVE: {
		//* echo back to client*/
		{
			char *buf = (char*)malloc(sizeof(char)*len + 1);
			memcpy(buf, in, len);
			buf[len] = '\0';
			
			lwsl_notice("[Main Service] Server recvived:%s\n", buf);

			sendMessageText(wsi, buf, strlen(buf));

			free(buf);
		}
	} break;
	case LWS_CALLBACK_CLOSED: {
		lwsl_notice("[Main Service] Client close.\n");
	} break;
	case LWS_CALLBACK_HTTP: {
		
	} break;
	case LWS_CALLBACK_SERVER_WRITEABLE: {
		sendMessagePingFrame(wsi);
	} break;
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] =
{
	{
		"lws-http",						/* name */
		http_service_callback,		/* callback */
		sizeof(struct per_session_data__http),	/* per_session_data_size */
		0,			/* max frame size / rx buffer */
		0,
		NULL,
		0
	},
	{
		"lws-ws-protocol",
		ws_service_callback,
		sizeof(struct session_data),
		0,
		0,
		NULL,
		0
	},
	{ NULL, NULL, 0, 0, 0, 0, 0 }	 /* End of list */ 
};

static const struct lws_extension exts[] = {
	{ NULL, NULL, NULL /* terminator */ }
};

int test_ws_server(int port)
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