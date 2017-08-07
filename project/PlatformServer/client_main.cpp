#include "client_main.h"

#if defined(WIN32) || defined(_WIN32)
#include <winsock2.h>
#endif // WIN32

#include <stdio.h>
#include <iostream>
#include <signal.h>
#include <thread>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/event_compat.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "command.h"
#include "platform.pb.h"

#include "socket_client.h"
#include "socket_session.h"

using namespace LW;

static SocketClient __g_client;

class ClientHandler : public ISocketSessionHanlder
{
private:
	SocketSession* _session;

public:
	ClientHandler() : _session(nullptr)
	{
	}

	virtual ~ClientHandler()
	{
	}

public:
	virtual int onSocketConnected(SocketSession* session) override
	{
		this->_session = session;

		return 0;
	}

	virtual int onSocketDisConnect(SocketSession* session) override
	{
		return 0;
	}

	virtual int onSocketTimeout(SocketSession* session) override
	{
		return 0;
	}

	virtual int onSocketError(SocketSession* session) override
	{
		return 0;
	}

public:
	virtual void onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize) override
	{
		switch (cmd)
		{
		case cmd_heart_beat:
		{
			platform::msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf("heartBeat[%d]\n", msg.time());
		} break;
		case cmd_platform_sc_userinfo:
		{
			platform::msg_userinfo_reponse msg;
			msg.ParseFromArray(buf, bufsize);
			printf("userid: %d age:%d sex:%d name:%s address:%s\n", msg.uid(),
				msg.age(), msg.sex(), msg.name().c_str(), msg.address().c_str());
		} break;
		default:
			break;
		}
	}
};

void run_rpc_client(lw_int32 port)
{
	if (__g_client.create(new ClientHandler))
	{
		for (int i = 0; i < 1; i++)
		{
			__g_client.getTimer()->start(100+i, 1+i, [](int id) -> bool
			{
				platform::msg_heartbeat msg;
				msg.set_time(time(NULL));
				lw_int32 len = (lw_int32)msg.ByteSizeLong();
				lw_char8 s[256] = { 0 };
				lw_bool ret = msg.SerializeToArray(s, len);
				if (ret)
				{
					__g_client.getSession()->sendData(cmd_heart_beat, s, len, cmd_heart_beat, [](lw_char8* buf, lw_int32 bufsize) -> bool
					{
						platform::msg_heartbeat msg;
						msg.ParseFromArray(buf, bufsize);
						printf("heartBeat[%d]\n", msg.time());

						return false;
					});
				}
				return true;
			});
		}
	
		int ret = __g_client.run("127.0.0.1", port);
	}
}

int __connect_center_server(const lw_char8* addr, const lw_char8* sport)
{
	lw_short16 port = std::atoi(sport);
	std::thread t(run_rpc_client, port);
	t.detach();

	return 0;
}

