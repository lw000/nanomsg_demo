#include "example.h"

#include <stdlib.h>

#include <libwebsockets.h>

int http_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
	switch (reason) {
	case LWS_CALLBACK_HTTP: {
		char buf[256];
		int n, c;
		printf("lws_http_serve: %s\n", (const char *)in);
		n = 0;
		c = lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n);
		while (c > 0) {
			lwsl_notice("URI Arg %d: %s\n", ++n, buf);
			c = lws_hdr_copy_fragment(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_URI_ARGS, n);
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
	case LWS_CALLBACK_CLIENT_RECEIVE: {
		{
			char *buf = (char*)malloc(sizeof(char)*len + 1);
			memcpy(buf, in, len);
			buf[len] = '\0';

			printf("rx %d '%s'\n", (int)len, (char *)buf);

			free(buf);
		}
	} break;
	case LWS_CALLBACK_HTTP_BODY: {
		printf("lws_http_serve: %s\n", (const char *)in);
	} break;
	case LWS_CALLBACK_HTTP_BODY_COMPLETION:{
		printf("LWS_CALLBACK_HTTP_WRITEABLE\n");
	} break;
	case LWS_CALLBACK_HTTP_FILE_COMPLETION:{
		printf("LWS_CALLBACK_HTTP_WRITEABLE\n");
	} break;
	case LWS_CALLBACK_HTTP_WRITEABLE:{
		printf("LWS_CALLBACK_HTTP_WRITEABLE\n");
	} break;
	default:
		break;
	}

	return 0;
}