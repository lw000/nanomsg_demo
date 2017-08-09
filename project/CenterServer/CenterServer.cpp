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

	SocketSession* s1 = Sessions[0];
	const SocketSession* s2 = Sessions[0];

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

	printf("join ([%d] host: %s, port:%d)\n", session->getSocket(), session->getHost().c_str(), session->getPort());
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
		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));
		lw_int32 c = (lw_int32)msg.ByteSize();
		std::unique_ptr<char[]> s(new char[c + 1]);
		lw_bool ret = msg.SerializeToArray(s.get(), c);
		if (ret)
		{
			session->sendData(cmd_heart_beat, s.get(), c);
		}
	} break;
	default:
		break;
	}
}
