#include "example.h"

#include <libwebsockets.h>

int http_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	switch (reason) {
	case LWS_CALLBACK_HTTP: {
		lwsl_info("lws_http_serve: %s\n", (const char *)in);
	} break;
	case LWS_CALLBACK_CLIENT_RECEIVE: {
		((char *)in)[len] = '\0';
		lwsl_info("rx %d '%s'\n", (int)len, (char *)in);
	} break;
	case LWS_CALLBACK_HTTP_BODY: {

	} break;
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:{

	} break;
	case LWS_CALLBACK_HTTP_FILE_COMPLETION:{

	} break;
	case LWS_CALLBACK_HTTP_WRITEABLE:{

	} break;
	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:{

	} break;
	default:
		break;
	}

	return 0;
}