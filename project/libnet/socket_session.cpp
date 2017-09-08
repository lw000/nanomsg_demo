#include "socket_session.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "socket_core.h"
#include "socket_processor.h"

#include "log4z.h"
#include <memory>
#include <thread>

#define RECV_BUFFER_SIZE	16*2

class CoreSocket {
public:
	static void __read_cb(struct bufferevent* bev, void* userdata) {
		SocketSession *session = (SocketSession*)userdata;
		session->__onRead();
	}

	static void __write_cb(struct bufferevent *bev, void *userdata) {
		SocketSession * session = (SocketSession*)userdata;
		session->__onWrite();
	}

	static void __event_cb(struct bufferevent *bev, short event, void *userdata) {
		SocketSession *session = (SocketSession*)userdata;
		session->__onEvent(event);
	}

	static void __on_socket_data_parse_cb(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata) {
		SocketSession *session = (SocketSession *)userdata;
		session->__onParse(cmd, buf, bufsize);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
SocketSession::SocketSession(AbstractSocketSessionHanlder* handler) {
	this->_handler = handler;
	this->_connected = false;
	this->_bev = nullptr;
	this->_port = -1;
	this->_c = SESSION_TYPE::NONE;
}

SocketSession::~SocketSession() {
	this->__reset();
}

int SocketSession::create(SESSION_TYPE c, SocketProcessor* processor, evutil_socket_t fd, short ev) {
	if (this->_c != c)
	{
		this->_c = c;
	}
	
	if (this->_processor != processor)
	{
		this->_processor = processor;
	}

	switch (this->_c) {
	case SESSION_TYPE::Server: {
		this->_bev = bufferevent_socket_new(this->_processor->getBase(), fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
		bufferevent_setcb(this->_bev, CoreSocket::__read_cb, CoreSocket::__write_cb, CoreSocket::__event_cb, this);

		bufferevent_enable(this->_bev, ev);
	
		this->_connected = true;
	} break;
	case SESSION_TYPE::Client: {
		struct sockaddr_in saddr;
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = inet_addr(this->_host.c_str());
		saddr.sin_port = htons(this->_port);

		this->_bev = bufferevent_socket_new(this->_processor->getBase(), fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
		int con = bufferevent_socket_connect(this->_bev, (struct sockaddr *)&saddr, sizeof(saddr));
		if (con >= 0) {
			bufferevent_setcb(this->_bev, CoreSocket::__read_cb, NULL, CoreSocket::__event_cb, this);
			
// 			struct timeval timeout_read;
// 			evutil_timerclear(&timeout_read);
// 			timeout_read.tv_sec = 1;
// 			timeout_read.tv_usec = 0;
// 			bufferevent_set_timeouts(this->_bev, &timeout_read, NULL);

			bufferevent_enable(this->_bev, ev);
		}
		else {
			this->_connected = false;
		}
	} break;	
	default:
		break;
	}
	return 0;
}

void SocketSession::destroy() {
	this->__reset();
}

void SocketSession::__reset() {
	this->_connected = false;
	this->_bev = nullptr;
	this->_handler = nullptr;
	this->_host.clear();
	this->_port = -1;
	this->_c = SESSION_TYPE::NONE;
}

void SocketSession::setConnTimeout(int s) {

}

void SocketSession::setRecvTimeout(int s) {

}

void SocketSession::setSendTimeout(int s) {

}

void SocketSession::setHost(const std::string& host) {
	this->_host = host;
}

std::string SocketSession::getHost() const {
	return this->_host;
}

void SocketSession::setPort(int port) {
	this->_port = port;
}

int SocketSession::getPort() const {
	return this->_port;
}

bool SocketSession::connected() { 
	return this->_connected;
}

std::string SocketSession::debug() {
	char buf[512];
	evutil_socket_t fd = bufferevent_getfd(this->_bev);
	sprintf(buf, "(fd:%d ip:%s, port:%d, c:%d, connected:%d)", fd, this->_host.c_str(), this->_port, this->_c, this->_connected);
	return std::string(buf);
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize) {
	if (this->_connected) {
		int c = this->_processor->getSocketCore()->send(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32 {
			int c = bufferevent_write(this->_bev, p->buf, p->size);
			return c;
		});
		return c;
	}
	return -1;
}

lw_int32 SocketSession::sendData(lw_int32 cmd, void* object, lw_int32 objectSize, lw_int32 recvcmd, SocketCallback cb) {
	if (this->_connected) {
		std::unordered_map<lw_int32, SocketCallback>::iterator iter = this->_cmd_event_map.find(recvcmd);
		if (iter == _cmd_event_map.end()) {
			this->_cmd_event_map.insert(std::pair<lw_int32, SocketCallback>(recvcmd, cb));
		}
		else {
			iter->second = cb;
		}
		int c = this->_processor->getSocketCore()->send(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32 {
			int c1 = bufferevent_write(this->_bev, p->buf, p->size);
			return c1;
		});
		return c;
	}
	return -1;
}

void SocketSession::__onWrite() {

}

void SocketSession::__onRead() {
	struct evbuffer *input = bufferevent_get_input(this->_bev);
	size_t input_len = evbuffer_get_length(input);

#if 0
	char recv_buf[RECV_BUFFER_SIZE + 1];
	::memset(recv_buf, 0x00, sizeof(recv_buf));
	while (input_len > 0) {
		size_t recv_len = bufferevent_read(this->_bev, recv_buf, RECV_BUFFER_SIZE);
		if (recv_len > 0) {
			if (this->_processor->getSocketCore()->parse(recv_buf, recv_len, CoreSocket::__on_socket_data_parse_cb, this) == 0) {

			}
		}
		else {
			lw_char8 buf[256];
			sprintf(buf, "SocketSession::__onRead [read_len: %d]", recv_len);
			LOGD(buf);
		}
		input_len -= recv_len;
	}
#else
	std::unique_ptr<lw_char8[]> recv_buf(new lw_char8[input_len]);
	while (input_len > 0) {
		size_t recv_len = bufferevent_read(this->_bev, recv_buf.get(), input_len); 
		if (recv_len > 0) {
			if (this->_processor->getSocketCore()->parse(recv_buf.get(), recv_len, CoreSocket::__on_socket_data_parse_cb, this) == 0) {

			}
		}
		else {
			lw_char8 buf[256];
			sprintf(buf, "SocketSession::__onRead [thread::id: %d, read_len: %d]", GetCurrentThreadId(), recv_len);
			LOGD(buf);
		}
		input_len -= recv_len;
	}
#endif
}

void SocketSession::__onParse(lw_int32 cmd, char* buf, lw_int32 bufsize) {
	SocketCallback cb = nullptr;
	if (!_cmd_event_map.empty()) {
		cb = this->_cmd_event_map.at(cmd);
	}
	
	bool goon = true;
	if (cb != nullptr) {
		goon = cb(buf, bufsize);
	}

	if (goon) {
		this->_handler->onSocketParse(this, cmd, buf, bufsize);
	}
}

void SocketSession::__onEvent(short ev) {
	if (ev & BEV_EVENT_CONNECTED) {
		this->_connected = true;
		this->_handler->onSocketConnected(this);
		return;
	}

	if (ev & BEV_EVENT_READING) {
		this->_handler->onSocketError(this);
	}
	else if (ev & BEV_EVENT_WRITING) {
		this->_handler->onSocketError(this);
	}
	else if (ev & BEV_EVENT_EOF) {
		this->_handler->onSocketDisConnect(this);
	}
	else if (ev & BEV_EVENT_TIMEOUT) {
		this->_handler->onSocketTimeout(this);
	}
	else if (ev & BEV_EVENT_ERROR) {
		this->_handler->onSocketError(this);
	}

	this->_connected = false;
	this->_bev = nullptr;
}