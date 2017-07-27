#ifndef __desk_h__
#define __desk_h__

#include "common_struct.h"
#include <vector>

class IDesk
{
public:
	virtual ~IDesk() {}

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

class Desk : public IDesk
{
public:
	Desk();
	virtual ~Desk();

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
	DESK_INFO desk_info;

private:
	std::vector<USER_INFO*> _users;
};

#endif	// !__desk_h__