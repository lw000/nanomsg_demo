#include "PlatformServer.h"

#include "command.h"
#include "platform.pb.h"

using namespace LW;

PlatformServerHandler::PlatformServerHandler()
{

}

PlatformServerHandler::~PlatformServerHandler()
{
	SESSIONS::iterator iter = sessions.begin();
	for (iter; iter != sessions.end(); ++iter)
	{
		SocketSession * pSession = *iter;
		delete pSession;
	}

	SESSIONS().swap(sessions);
}

void PlatformServerHandler::onJoin(SocketSession* session)
{
	sessions.push_back(session);
	printf("join ([%d] host: %s, port:%d)\n", session->getSocket(), session->getHost().c_str(), session->getPort());
}

int PlatformServerHandler::onConnected(SocketSession* session)
{
	return 0;
}

int PlatformServerHandler::onDisConnect(SocketSession* session)
{
	return 0;
}

int PlatformServerHandler::onSocketTimeout(SocketSession* session)
{
	return 0;
}

int PlatformServerHandler::onSocketError(int error, SocketSession* session)
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

void PlatformServerHandler::onParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize)
{
	switch (cmd)
	{
	case cmd_heart_beat:
	{
		platform::msg_heartbeat msg;
		lw_llong64 t = time(NULL);
		msg.set_time(t);
		int len = msg.ByteSize();
		{
			char *s = new char[len + 1];
			bool ret = msg.SerializeToArray(s, len);
			if (ret)
			{
				session->sendData(cmd_heart_beat, s, len);
			}
			delete s;
		}
	} break;
	case cmd_platform_cs_userinfo:
	{
		platform::msg_userinfo_request client_userinfo;
		client_userinfo.ParseFromArray(buf, bufsize);

		platform::msg_userinfo_reponse userinfo;
		userinfo.set_uid(client_userinfo.uid());
		userinfo.set_age(30);
		userinfo.set_sex(1);
		userinfo.set_name("liwei");
		userinfo.set_address("guangdong");
		int len = userinfo.ByteSize();
		{
			char *s = new char[len + 1];
			bool ret = userinfo.SerializePartialToArray(s, len);
			if (ret)
			{
				session->sendData(cmd_platform_sc_userinfo, s, strlen(s));
			}
			delete s;
		}
	} break;
	default:
		break;
	}
}
