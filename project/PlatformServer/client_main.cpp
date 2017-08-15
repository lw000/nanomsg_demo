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

#include "socket_processor.h"
#include "socket_client.h"
#include "socket_session.h"
#include "socket_timer.h"

using namespace LW;

class ClientHandler : public ISocketSessionHanlder
{
public:
	ClientHandler()
	{
	}

	virtual ~ClientHandler()
	{
	}

public:
	virtual int onSocketConnected(SocketSession* session) override
	{
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
		case cmd_connected:
		{
			platform::msg_connected msg;
			msg.ParseFromArray(buf, bufsize);
			printf("connected. [time : %d]\n", msg.time());

		} break;
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

//////////////////////////////////////////////////////////////////////////////////////////

static SocketProcessor __g_processor;
static SocketClient __g_client;
static Timer __g_timer;

int __connect_center_server(const lw_char8* addr, const lw_char8* sport)
{
	lw_short16 port = std::atoi(sport);
	if (__g_client.create(&__g_processor, new ClientHandler()))
	{
		__g_timer.create(&__g_processor);
		__g_timer.start(100, 2000, [](int tid, unsigned int tms) -> bool
		{
			platform::msg_heartbeat msg;
			msg.set_time(time(NULL));
			lw_int32 c = (lw_int32)msg.ByteSizeLong();
			std::unique_ptr<char[]> s(new char[c + 1]()); 
			lw_bool ret = msg.SerializeToArray(s.get(), c);
			if (ret)
			{
// 				__g_client.getSession()->sendData(cmd_heart_beat, s.get(), c, cmd_heart_beat, [](lw_char8* buf, lw_int32 bufsize) -> bool
// 				{
// 					platform::msg_heartbeat msg;
// 					msg.ParseFromArray(buf, bufsize);
// 					printf("heartBeat[%d]\n", msg.time());
// 
// 					return false;
// 				});

				__g_client.getSession()->sendData(cmd_heart_beat, s.get(), c);
			}
			return true;
		});

		int ret = __g_client.run("127.0.0.1", port);
	}

	return 0;
}

