#ifndef __nanomsg_socket_h__
#define __nanomsg_socket_h__

#include <stddef.h>
#include "common_type.h"

class SocketCore;

class NanomsgSocket
{
	friend class CoreSocket;

public:
	NanomsgSocket();
	virtual ~NanomsgSocket();

public:
	int create(int domain, unsigned int protocol);
	int bind(const char *addr);
	int connect(const char *addr);
	int setsockopt(int level, int option, const void *optval, size_t optvallen);
	int getsockopt(int level, int option, void *optval, size_t *optvallen);
	int shutdown();
	int close();

public:
	int send(int cmd, void* object, int objectSize);
	int sendmsg(int cmd, void* object, int objectSize, const struct nn_msghdr *msghdr, int flags);
	int recv();
	int recvmsg(struct nn_msghdr *msghdr, int flags);

private:
	virtual void onRecv(lw_int32 cmd, char* buf, lw_int32 bufsize) = 0;

private:
	int _fd;
	SocketCore* _core;
};

#endif

