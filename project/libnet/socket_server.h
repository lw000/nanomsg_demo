#ifndef __SocketServer_H__
#define __SocketServer_H__

#include <vector>
#include <list>
#include <unordered_map>
#include <event2/util.h>
#include "object.h"

#include "socket_hanlder.h"

#include <functional>

class Timer;
class SocketSession;
class SocketProcessor;
struct evconnlistener;

class SocketServer : public Object
{
public:
	SocketServer();
	virtual ~SocketServer();

public:
	bool create(SocketProcessor* processor, ISocketServerHandler* isession);
	void destroy();

public:
	lw_int32 run(u_short port, std::function<void(lw_int32 what)> func);

public:
	int getPort() const { return this->_port; }

public:
	void listener_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int);
	void listener_error_cb(struct evconnlistener *);

public:
	virtual std::string debug() override;

private:
	struct evconnlistener * __createConnListener(int port);

private:
	void __run();

private:
	SocketProcessor* _processor;
	lw_int32 _port;
	Timer* _timer;

private:
	std::function<void(lw_int32 what)> _onFunc;
	ISocketServerHandler* iserver;
};

#endif // !__SocketServer_H__ 
