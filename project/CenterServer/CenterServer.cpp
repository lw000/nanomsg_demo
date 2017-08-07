#include "CenterServer.h"

#include "command.h"
#include "platform.pb.h"

using namespace LW;

ServerHandler::ServerHandler()
{

}

ServerHandler::~ServerHandler()
{
	
}

void ServerHandler::onListener(SocketSession* session)
{
	Sessions.add(session);
	SocketSession* s = Sessions.find([](SocketSession* s) -> bool
	{
		return true;
	});

	printf("join ([%d] host: %s, port:%d)\n", session->getSocket(), session->getHost().c_str(), session->getPort());
}

int ServerHandler::onSocketConnected(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketDisConnect(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketTimeout(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketError(SocketSession* session)
{
	Sessions.remove(session);

	return 0;
}

void ServerHandler::onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize)
{
	switch (cmd)
	{
	case cmd_heart_beat:
	{
		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));
		lw_int32 len = (lw_int32)msg.ByteSize();
		{
			lw_char8 *s = new lw_char8[len + 1];
			lw_bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				session->sendData(cmd_heart_beat, s, len);
			}
			delete s;
		}
	} break;
	default:
		break;
	}
}
