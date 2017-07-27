#ifndef __usermgr_h__
#define __usermgr_h__

#include <list>
#include <mutex>

#include "common_struct.h"

class UserMgr
{
private:
	std::mutex _m;

private:
	std::list<USER_INFO*> _live_users;	// 用户
	std::list<USER_INFO*> _die_users;	// 用户

public:
	UserMgr();
	~UserMgr();

public:
	USER_INFO* getUser(int uid);
	void addUser(const USER_INFO* user);
	void removeUser(int uid);

public:
	void removeUserTest();

public:
	void restoreCache();

};

#endif	// !__usermgr_h__
