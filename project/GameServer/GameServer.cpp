#include "GameServer.h"

#include "socket_processor.h"
#include "socket_session.h"
#include "socket_client.h"
#include "socket_timer.h"

#include "command.h"
#include "platform.pb.h"
#include "game.pb.h"
#include <iostream>

using namespace LW;

////////////////////////////////////////////////////////////////////////////////////////////////////

GameServer::GameServer(IGameServer* idesk) : iDesk(idesk)
{
	_processor = new SocketProcessor;
	timer = new Timer();
	client = new SocketClient();
}

GameServer::~GameServer()
{
	this->destroy();

	if (timer == nullptr)
	{
		delete timer;
		timer = nullptr;
	}
	
	if (client != nullptr)
	{
		delete client;
		client = nullptr;
	}
}

bool GameServer::create(const DESK_INFO& info)
{
	this->_desk_info = info;

	bool ret = false;
	do 
	{
		if (!client->create(_processor, this)) break;

		timer->create(_processor);

		ret = true;

	} while (0);

	return ret;
}

void GameServer::destroy()
{
	timer->destroy();
}

void GameServer::start(const std::string& host, int port)
{
	do 
	{
		timer->start(100, 1000, [this](int tid, unsigned int tms) -> bool
		{
			platform::msg_userinfo_request msg;
			msg.set_uid(400000);
			int c = msg.ByteSizeLong();
			std::unique_ptr<char[]> s(new char[c + 1]());
			bool ret = msg.SerializeToArray(s.get(), c);
			if (ret)
			{
				client->getSession()->sendData(cmd_platform_cs_userinfo, s.get(), c);
			}
			return true;
		});
		
		int ret = client->run(host, port);
	} while (0);
}

void GameServer::sendData(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	client->getSession()->sendData(cmd, object, objectSize);
}

int GameServer::onStart()
{
	printf("onStart.\n");
	return 0;
}

int GameServer::onEnd()
{
	printf("onEnd.\n");
	return 0;
}

int GameServer::onSocketConnected(SocketSession* session)
{
	return 0;
}

int GameServer::onSocketDisConnect(SocketSession* session)
{
	client->loopbreak();
	return 0;
}

int GameServer::onSocketTimeout(SocketSession* session)
{
	client->loopbreak();
	return 0;
}

int GameServer::onSocketError(SocketSession* session)
{
	client->loopbreak();
	return 0;
}

void GameServer::onSocketParse(SocketSession* session, lw_int32 cmd, lw_char8* buf, lw_int32 bufsize)
{
	switch (cmd)
	{
	case cmd_connected:
	{
		printf("connected  [port:%d]\n", client->getSession()->getSocket());
	} break;
	case cmd_game_frame_cs_game_start:
	case cmd_game_frame_sc_game_end:
	case cmd_game_frame_sc_sit_up:
	case cmd_game_frame_sc_sit_down:
	{
		this->frameMessage(cmd, buf, bufsize);
	} break;
	default:
	{
		this->onGameMessage(cmd, buf, bufsize);
	} break;
	}
}

int GameServer::frameMessage(int cmd, void* data, int datasize)
{
	switch (cmd)
	{
	case cmd_game_frame_cs_game_start:
	{
		this->onGameStartReponse(data, datasize);
	} break;
	case cmd_game_frame_sc_game_end:
	{
		this->onGameEndReponse(data, datasize);
	} break;
	case cmd_game_frame_sc_sit_up:
	{
		this->onGameUserSitupReponse(data, datasize);
	} break;
	case cmd_game_frame_sc_sit_down:
	{
		this->onGameUserSitdownReponse(data, datasize);
	} break;
	default:
		break;
	}

	return 0;
}

void GameServer::onGameStartReponse(void* data, int datasize)
{
	msg::game_msg_start_reponse gstart;
	gstart.ParseFromArray(data, datasize);
	printf("game_msg_start_reponse: state: %d\n", gstart.state());
}

void GameServer::onGameEndReponse(void* data, int datasize)
{
	msg::game_msg_end_reponse gend;
	gend.ParseFromArray(data, datasize);
	printf("game_msg_end_reponse: state: %d\n", gend.state());
}

void GameServer::onGameUserSitupReponse(void* data, int datasize)
{
	msg::game_msg_situp_reponse situp;
	situp.ParseFromArray(data, datasize);
	printf("game_msg_situp_reponse uid: %d seat:%d lookup:%d\n", situp.uid(), situp.seat(), situp.lookup());
}

void GameServer::onGameUserSitdownReponse(void* data, int datasize)
{
	msg::game_msg_sitdown_reponse sitdown;
	sitdown.ParseFromArray(data, datasize);
	printf("game_msg_sitdown_reponse uid: %d seat:%d lookup: %d\n", sitdown.uid(), sitdown.seat(), sitdown.lookup());
}

int GameServer::onGameMessage(int cmd, void* data, int datasize)
{
	switch (cmd)
	{
	case cmd_platform_sc_userinfo:
	{
		platform::msg_userinfo_reponse userinfo;
		userinfo.ParseFromArray(data, datasize);
		printf("userid: %d age:%d sex:%d name:%s address:%s ext:%s\n", userinfo.uid(),
			userinfo.age(), userinfo.sex(), userinfo.name().c_str(), userinfo.address().c_str(), userinfo.ext().c_str());
	}break;
	default:
		break;
	}

	return 0;
}

void GameServer::sendSitup()
{
	msg::game_msg_situp_request situp;
	situp.set_uid(1);
	int c = situp.ByteSizeLong();
	std::unique_ptr<char[]> s(new char[c + 1]);
	bool ret = situp.SerializeToArray(s.get(), c);
	client->getSession()->sendData(cmd_game_frame_cs_sit_up, s.get(), c);
}

void GameServer::sendSitdown()
{
	msg::game_msg_sitdown_request sitdown;
	sitdown.set_uid(1);
	int c = sitdown.ByteSizeLong();
	std::unique_ptr<char[]> s(new char[c + 1]);
	bool ret = sitdown.SerializeToArray(s.get(), c);
	client->getSession()->sendData(cmd_game_frame_cs_sit_down, s.get(), c);
}
