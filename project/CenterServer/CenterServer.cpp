#include "CenterServer.h"

#include "command.h"
#include "platform.pb.h"

using namespace LW;

ServerHandler::ServerHandler()
{

}

ServerHandler::~ServerHandler()
{
	SESSIONS::iterator iter = sessions.begin();
	for (iter; iter != sessions.end(); ++iter)
	{
		SocketSession * pSession = *iter;
		delete pSession;
	}

	SESSIONS().swap(sessions);
}

void ServerHandler::onJoin(SocketSession* session)
{
	sessions.push_back(session);
	printf("join ([%d] host: %s, port:%d)\n", session->getSocket(), session->getHost().c_str(), session->getPort());
}

int ServerHandler::onConnected(SocketSession* session)
{
	return 0;
}

int ServerHandler::onDisConnect(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketTimeout(SocketSession* session)
{
	return 0;
}

int ServerHandler::onSocketError(int error, SocketSession* session)
{
	SESSIONS::iterator iter = sessions.begin();
	while (iter != sessions.end())
	{
		SocketSession *pSession = *iter;
		if (pSession == session)
		{
			printf("leave host=%s, port=%d\n", pSession->getHost().c_str(), pSession->getPort());

			delete pSession;
			pSession = NULL;

			iter = sessions.erase(iter);
			break;
		}
		++iter;
	}

	return 0;
}

void ServerHandler::onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize)
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
