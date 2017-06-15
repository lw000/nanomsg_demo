#include "client_http.h"

#include "event2/event-config.h"

#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/buffer.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <thread>

#define VERIFY(cond) do {         \
	if (!(cond)) {                              \
		fprintf(stderr, "[error] %s\n", #cond); \
	}                                           \
} while (0);                                    \

#define URL_MAX 4096

struct connect_base
{
	struct evhttp_connection *evcon;
	struct evhttp_uri *host;
};

static void get_cb(struct evhttp_request *req, void *arg)
{
	struct evbuffer *evbuf;

	VERIFY(req);

	char buf[1024];

	evbuf = evhttp_request_get_input_buffer(req);
	ev_ssize_t len = evbuffer_get_length(evbuf);
	buf[len] = '\0';
	memcpy(buf, evbuffer_pullup(evbuf, len), len);
	printf("%s \n", buf);

	evbuffer_drain(evbuf, len);

	evhttp_request_free(req);
}

static void connect_cb(struct evhttp_request *proxy_req, void *arg)
{
	char buffer[URL_MAX];

	struct connect_base *base = (struct connect_base *)arg;
	struct evhttp_connection *evcon = base->evcon;
	struct evhttp_uri *host_url = base->host;

	VERIFY(proxy_req);
	if (evcon)
	{
		struct evhttp_request *req = evhttp_request_new(get_cb, NULL);
		evhttp_add_header(req->output_headers, "Connection", "close");
		VERIFY(!evhttp_make_request(evcon, req, EVHTTP_REQ_GET, evhttp_uri_join(host_url, buffer, URL_MAX)));
	}
}

void __run_http_client()
{
	char argv[][256] = {
		"http://127.0.0.1:9877/add?a=111&b=2222",
	};

	char buffer[URL_MAX];

	struct evhttp_uri *host;

	struct event_base *base;
	struct evhttp_connection *evcon;
	struct evhttp_request *req;

	struct connect_base connect_base;

	host = evhttp_uri_parse(argv[0]);

	VERIFY(evhttp_uri_get_host(host));
	VERIFY(evhttp_uri_get_port(host) > 0);

	VERIFY(base = event_base_new());

	VERIFY(evcon = evhttp_connection_base_new(base, NULL,
		evhttp_uri_get_host(host), evhttp_uri_get_port(host)));

	connect_base.evcon = evcon;
	connect_base.host = host;

	VERIFY(req = evhttp_request_new(connect_cb, &connect_base));

	evhttp_add_header(req->output_headers, "Connection", "keep-alive");
	evhttp_add_header(req->output_headers, "Proxy-Connection", "keep-alive");
	evutil_snprintf(buffer, URL_MAX, "%s:%d",
		evhttp_uri_get_host(host), evhttp_uri_get_port(host));
	evhttp_make_request(evcon, req, EVHTTP_REQ_CONNECT, buffer);

	event_base_dispatch(base);

	evhttp_connection_free(evcon);
	event_base_free(base);
	evhttp_uri_free(host);
}

int run_client_http(lw_int32 exe_times)
{
	for (size_t i = 0; i < exe_times; i++)
	{
		std::thread t(__run_http_client);
		t.detach();
	}

	return 0;
}
