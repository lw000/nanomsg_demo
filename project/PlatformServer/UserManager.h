#ifndef __usermgr_h__
#define __usermgr_h__

#include <list>
#include <mutex>

#include "common_struct.h"

class SocketSession;

struct UserSession
{
	USER_INFO user;
	SocketSession* session;
};

class IUserManager
{
public:
	virtual ~IUserManager() {}

public:
	virtual UserSession* get(int uid) = 0;
	virtual int add(const USER_INFO& user, SocketSession* session) = 0;
	virtual void remove(const SocketSession* session) = 0;
	virtual void remove(int uid) = 0;
};

class UserManager : public IUserManager
{
public:
	UserManager();
	virtual ~UserManager();
	
public:
	virtual UserSession* get(int uid) override;
	virtual int add(const USER_INFO& user, SocketSession* session) override;
	virtual void remove(const SocketSession* session) override;
	virtual void remove(int uid) override;

public:
	void removeUserTest();

public:
	void restoreCache();

private:
	std::mutex _m;

private:
	std::list<UserSession*> _live;	// 激活用户
	std::list<UserSession*> _die;	// 用户
};

#endif	// !__usermgr_h__
