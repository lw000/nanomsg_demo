#include "UserMgr.h"
#include <algorithm>
#include "..\libcrossLog\log4z.h"
using namespace zsummer::log4z;

UserMgr::UserMgr()
{
}


UserMgr::~UserMgr()
{
	{
		std::list<USER_INFO*>::iterator iter = _live_users.begin();
		for (; iter != _live_users.end(); ++iter)
		{
			USER_INFO* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}

	{
		std::list<USER_INFO*>::iterator iter = _die_users.begin();
		for (; iter != _die_users.end(); ++iter)
		{
			USER_INFO* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}
}

USER_INFO* UserMgr::getUser(int uid)
{
	{
		std::lock_guard < std::mutex > l(_m);
		std::list<USER_INFO*>::iterator iter = _live_users.begin();
		for (; iter != _live_users.end(); ++iter)
		{
			if (uid == (*iter)->id)
			{
				return *iter;
			}
		}
	}
	
	return NULL;
}

void UserMgr::addUser(const USER_INFO* user)
{
	// 如果用户不存在，创建对象，添加用户
	// 如果用户存在，更新用户信息

	{
		std::lock_guard < std::mutex > l(_m);
		USER_INFO* pUserinfo = NULL;
		std::list<USER_INFO*>::iterator iter = _live_users.begin();
		for (; iter != _live_users.end(); ++iter)
		{
			if (user->id == (*iter)->id)
			{
				pUserinfo = *iter; break;
			}
		}

		if (pUserinfo == NULL)
		{
			{
				// 缓存中取出对象
				if (!_die_users.empty())
				{
					pUserinfo = _die_users.front();
					_die_users.pop_front();
				}
			}
			
			if (pUserinfo == NULL)
			{
				pUserinfo = new USER_INFO;
			}

			*pUserinfo = *user;

			_live_users.push_back(const_cast<USER_INFO*>(pUserinfo));

// 			{
// 				char s[512];
// 				sprintf(s, "add: [%d]", pUserinfo->u_id);
// 				LOGD(s);
// 			}
		}
		else
		{
			*pUserinfo = *user;

// 			{
// 				char s[512];
// 				sprintf(s, "--------------------------------->update: [%d]", pUserinfo->u_id);
// 				LOGD(s);
// 			}
		}
	}
}

void UserMgr::removeUser(int uid)
{
	{
		std::lock_guard < std::mutex > l(_m);
		USER_INFO* pUserinfo = NULL;
		std::list<USER_INFO*>::iterator iter = _live_users.begin();
		for (; iter != _live_users.end(); ++iter)
		{
			if (uid == (*iter)->id)
			{
				pUserinfo = *iter;
				_live_users.erase(iter);
				break;
			}
		}

		if (pUserinfo != nullptr)
		{
			_die_users.push_back(pUserinfo);

// 			{
// 				char s[512];
// 				sprintf(s, "---------->remove: [%d]", pUserinfo->u_id);
// 				LOGD(s);
// 			}
		}
	}
}

void UserMgr::removeUserTest()
{
	{
		std::lock_guard < std::mutex > l(_m);
		USER_INFO* pUserinfo = NULL;

		// 缓存中取出对象
		if (!_live_users.empty())
		{
			pUserinfo = _live_users.front();
			_live_users.pop_front();
		}

		if (pUserinfo != nullptr)
		{
			_die_users.push_back(pUserinfo);
			{
				char s[512];
				sprintf(s, "---------->remove: [%d]", pUserinfo->id);
				LOGD(s);
			}
		}
	}
}

void UserMgr::restoreCache()
{
	{
		std::lock_guard < std::mutex > l(_m);

		{
			std::list<USER_INFO*>::iterator iter = _die_users.begin();
			for (; iter != _die_users.end(); ++iter)
			{
				USER_INFO* pUserinfo = (*iter);
				delete pUserinfo;
			}

			std::list<USER_INFO*>().swap(_die_users);
		}
	}
}

