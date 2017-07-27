#include "CenterServer.h"

#include "command.h"
#include "platform.pb.h"

using namespace LW;

CenterServerHandler::CenterServerHandler()
{

}

CenterServerHandler::~CenterServerHandler()
{
	SESSIONS::iterator iter = sessions.begin();
	for (iter; iter != sessions.end(); ++iter)
	{
		SocketSession * pSession = *iter;
		delete pSession;
	}

	SESSIONS().swap(sessions);
}

void CenterServerHandler::onJoin(SocketSession* session)
{
	sessions.push_back(session);
	printf("join ([%d] host: %s, port:%d)\n", session->getSocket(), session->getHost().c_str(), session->getPort());
}

int CenterServerHandler::onConnected(SocketSession* session)
{
	return 0;
}

int CenterServerHandler::onDisConnect(SocketSession* session)
{
	return 0;
}

int CenterServerHandler::onSocketTimeout()
{
	return 0;
}

int CenterServerHandler::onSocketError(int error, SocketSession* session)
{
	SESSIONS::iterator iter = sessions.begin();
	while (iter != sessions.end())
	{
		SocketSession * pSession = *iter;
		if (pSession == session)
		{
			printf("leave host=%s, port=%d\n", pSession->getHost().c_str(), pSession->getPort());

			delete pSession;
			iter = sessions.erase(iter);
			break;
		}
		++iter;
	}

	return 0;
}

void CenterServerHandler::onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize)
{
	switch (cmd)
	{
	case cmd_heart_beat:
	{
		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));

		lw_int32 len = (lw_int32)msg.ByteSizeLong();
		lw_char8 s[256] = { 0 };
		lw_bool ret = msg.SerializeToArray(s, len);
		if (ret)
		{
			session->sendData(cmd_heart_beat, s, len);
		}
	} break;
	default:
		break;
	}
}
