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

public:
	UserSession()
	{
		this->reset();
	}

	void reset()
	{
		session = nullptr;
		::memset(&user, 0x00, sizeof(user));
	}
};

class AbstractUser
{
public:
	virtual ~AbstractUser() {}

public:
	virtual int add(const USER_INFO& user, SocketSession* session) = 0;

public:
	virtual const UserSession* find(int uid) = 0;
	virtual const UserSession* find(const USER_INFO* user) = 0;
	virtual const UserSession* find(const SocketSession* session) = 0;

public:
	virtual void remove(int uid) = 0;
	virtual void remove(const USER_INFO* user) = 0;
	virtual void remove(const SocketSession* session) = 0;

// public:
// 	virtual void update() = 0;
};

class Users : public AbstractUser
{
	typedef std::list<UserSession*> USER_LIST;
	typedef USER_LIST::iterator iterator;
	typedef USER_LIST::const_iterator const_iterator;

public:
	Users();
	virtual ~Users();

public:
	UserSession* operator[](int i);

public:
	virtual const UserSession* find(int uid) override;
	virtual const UserSession* find(const USER_INFO* user) override;
	virtual const UserSession* find(const SocketSession* session) override;

public:
	virtual int add(const USER_INFO& user, SocketSession* session) override;

public:
	virtual void remove(const SocketSession* session) override;
	virtual void remove(int uid) override;
	virtual void remove(const USER_INFO* user) override;

public:
	void removeUserTest();

public:
	void restoreCache();

private:
	std::mutex _m;

private:
	USER_LIST _live;	// 激活用户
	USER_LIST _cache;	// 缓存用户
};

#endif	// !__usermgr_h__
