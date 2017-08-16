#include "CenterServer.h"

#include "socket_session.h"

#include "command.h"
#include "platform.pb.h"

#include <iostream>

using namespace LW;

ServerHandler::ServerHandler()
{

}

ServerHandler::~ServerHandler()
{
	
}

void ServerHandler::sendHeartbeat(SocketSession* session)
{
	platform::msg_heartbeat msg;
	msg.set_time(time(NULL));
	lw_int32 c = (lw_int32)msg.ByteSize();
	std::unique_ptr<char[]> s(new char[c + 1]);
	lw_bool ret = msg.SerializeToArray(s.get(), c);
	if (ret)
	{
		session->sendData(cmd_heart_beat, s.get(), c);
	}
}

int ServerHandler::onStart()
{
	return 0;
}

int ServerHandler::onEnd()
{
	return 0;
}

void ServerHandler::onListener(SocketSession* session)
{
	Sessions.add(session);

	platform::msg_connected msg;
	lw_llong64 t = time(NULL);
	msg.set_time(t);
	int len = msg.ByteSize();
	{
		char *s = new char[len + 1];

		bool ret = msg.SerializeToArray(s, len);
		if (ret)
		{
			session->sendData(cmd_connected, s, len);
		}
		delete s;
	}

	std::cout << *session << std::endl;
}

int ServerHandler::onSocketConnected(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketDisConnect(SocketSession* session)
{
	Sessions.remove(session);
	return 0;
}

int ServerHandler::onSocketTimeout(SocketSession* session)
{
	Sessions.remove(session);
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
		sendHeartbeat(session);	
	} break;
	default:
		break;
	}
}
