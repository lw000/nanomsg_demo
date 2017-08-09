#ifndef __GameLogic_H__
#define __GameLogic_H__

#include <string>
#include <vector>

#include "base_type.h"
#include "common_struct.h"
#include "socket_session.h"
#include "socket_client.h"

class EventObject;
class Timer;

class IGameServer
{
public:
	virtual ~IGameServer() {}

public:
	virtual bool create(const DESK_INFO& info) = 0;
	virtual void destroy() = 0;

public:
	virtual void onGameStartReponse(void* data, int datasize) = 0;
	virtual void onGameEndReponse(void* data, int datasize) = 0;
	virtual void onGameUserSitupReponse(void* data, int datasize) = 0;
	virtual void onGameUserSitdownReponse(void* data, int datasize) = 0;

public:
	virtual int onGameMessage(int cmd, void* data, int datasize) = 0;
};

class GameServer : public IGameServer, public ISocketClient
{
private:
	IGameServer* iDesk;

private:
	DESK_INFO _desk_info;
	std::vector<USER_INFO> users;

public:
	GameServer(IGameServer* idesk);
	virtual ~GameServer();

public:
	void start(const std::string& host, int port);

public:
	void sendSitup();
	void sendSitdown();

private:
	void sendData(lw_int32 cmd, void* object, lw_int32 objectSize);

protected:
	virtual int onSocketConnected() override;
	virtual int onSocketDisConnect() override;
	virtual int onSocketTimeout() override;
	virtual int onSocketError() override;

	virtual void onSocketParse(lw_int32 cmd, lw_char8* buf, lw_int32 bufsize);

public:
	int frameMessage(int cmd, void* data, int datasize);

public:
	virtual bool create(const DESK_INFO& info) override;
	virtual void destroy() override;

public:
	virtual void onGameStartReponse(void* data, int datasize) override;
	virtual void onGameEndReponse(void* data, int datasize) override;
	virtual void onGameUserSitupReponse(void* data, int datasize) override;
	virtual void onGameUserSitdownReponse(void* data, int datasize) override;

public:
	virtual int onGameMessage(int cmd, void* data, int datasize) override;

private:
	SocketClient* client;
	EventObject* _evObject;
	Timer* timer;
};

#endif	//__GameLogic_H__

