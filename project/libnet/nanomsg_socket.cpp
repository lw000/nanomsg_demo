#include "nanomsg_socket.h"

#include "socket_core.h"

#include <nn.h>

#include <log4z.h>

class CoreSocket
{
public:
	static void __on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata) {
		NanomsgSocket * c = (NanomsgSocket*)userdata;
		c->onRecv(cmd, buf, bufsize);
	}
};

NanomsgSocket::NanomsgSocket() {
	this->_fd = -1;
	_core = new SocketCore;
}

NanomsgSocket::~NanomsgSocket() {
	delete _core;
	close();
}

int NanomsgSocket::create(int domain, unsigned int protocol) {
	this->_fd = nn_socket(domain, protocol);
	return this->_fd;
}

int NanomsgSocket::bind(const char *addr) {
	int r = nn_bind(this->_fd, addr);
	if (r < 0)
	{
		nn_close(this->_fd);

		lw_char8 buf[512];
		sprintf(buf, "nn_bind: %s\n", nn_strerror(nn_errno()));
		LOGD(buf);

		return (-1);
	}
	return 0;
}

int NanomsgSocket::connect(const char *addr) {
	int r = nn_connect(this->_fd, addr);
	if (r < 0) {
		nn_close(this->_fd);

		lw_char8 buf[512];
		sprintf(buf, "nn_connect: %s\n", nn_strerror(nn_errno()));
		LOGD(buf);

		return (-1);
	}
	return 0;
}

int NanomsgSocket::setsockopt(int level, int option, const void *optval, size_t optvallen) {
	int r = nn_setsockopt(this->_fd, level, option, optval, optvallen);
	if (r < 0) {
		nn_close(this->_fd);

		lw_char8 buf[512];
		sprintf(buf, "nn_setsockopt: %s\n", nn_strerror(nn_errno()));
		LOGD(buf);
		return (-1);
	}

	return 0;
}

int NanomsgSocket::getsockopt(int level, int option, void *optval, size_t *optvallen) {
	int r = nn_getsockopt(this->_fd, level, option, optval, optvallen);
	return r;
}

int NanomsgSocket::close() {
	int r = nn_close(this->_fd);
	return r;
}

int NanomsgSocket::shutdown() {
	int r = nn_shutdown(this->_fd, 0);
	return r;
}

int NanomsgSocket::recv() {
	char *buf = NULL;
	lw_int32 c = nn_recv(this->_fd, &buf, NN_MSG, 0);
	if (c > 0) {
		_core->parse(buf, c, CoreSocket::__on_socket_recv, this);
		nn_freemsg(buf);
	}
	return c;
}

int NanomsgSocket::send(int cmd, void* object, int objectSize) {
	int c = _core->send(cmd, object, objectSize, [this](LW_NET_MESSAGE * p) -> lw_int32
	{
		int c = nn_send(this->_fd, p->buf, p->size, 0);
		return c;
	});
	return c;
}

int NanomsgSocket::sendmsg(int cmd, void* object, int objectSize, const struct nn_msghdr *msghdr, int flags) {

	return 0;
}

int NanomsgSocket::recvmsg(struct nn_msghdr *msghdr, int flags) {
	return 0;
}
