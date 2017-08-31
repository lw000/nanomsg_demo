
#ifndef __example_h__
#define __example_h__

extern int test_ws_client(int port);

extern int test_ws_server(int port);

extern int http_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

#endif	// !__example_h__