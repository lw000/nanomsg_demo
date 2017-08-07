#include "UserManager.h"
#include <algorithm>

#include "..\libcrossLog\log4z.h"

#include "socket_session.h"

using namespace zsummer::log4z;

UserManager::UserManager()
{
}


UserManager::~UserManager()
{
	{
		std::list<UserSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			UserSession* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}

	{
		std::list<UserSession*>::iterator iter = _die.begin();
		for (; iter != _die.end(); ++iter)
		{
			UserSession* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}
}

UserSession* UserManager::get(int uid)
{
	UserSession* pUserinfo = nullptr;
	{
		std::lock_guard < std::mutex > l(_m);
		std::list<UserSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (uid == (*iter)->user.id)
			{
				pUserinfo = *iter;
				break;
			}
		}
	}
	
	return pUserinfo;
}

int UserManager::add(const USER_INFO& user, SocketSession* session)
{
	// 如果用户不存在，创建对象，添加用户
	// 如果用户存在，更新用户信息
	int r = 0;

	UserSession* pUsession = nullptr;

	{
		std::lock_guard < std::mutex > l(_m);
		std::list<UserSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (user.id == (*iter)->user.id)
			{
				pUsession = *iter; break;
			}
		}

		if (pUsession == nullptr)
		{
			// 缓存中取出对象
			if (!_die.empty())
			{
				pUsession = _die.front();
				_die.pop_front();
			}
			
			if (pUsession == nullptr)
			{
				pUsession = new UserSession;
			}
			_live.push_back(pUsession);
		}

		pUsession->session = session;
		pUsession->user = user;
	}

	return r;
}

void UserManager::remove(const SocketSession* session)
{
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* usession = nullptr;
		std::list<UserSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (session == (*iter)->session)
			{
				usession = *iter;
				_live.erase(iter);
				break;
			}
		}

		if (usession != nullptr)
		{
			_die.push_back(usession);
		}
	}
}

void UserManager::remove(int uid)
{
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* pUserinfo = NULL;
		std::list<UserSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (uid == (*iter)->user.id)
			{
				pUserinfo = *iter;
				_live.erase(iter);
				break;
			}
		}

		if (pUserinfo != nullptr)
		{
			_die.push_back(pUserinfo);

// 			{
// 				char s[512];
// 				sprintf(s, "---------->remove: [%d]", pUserinfo->u_id);
// 				LOGD(s);
// 			}
		}
	}
}

void UserManager::removeUserTest()
{
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* pUserinfo = NULL;

		// 缓存中取出对象
		if (!_live.empty())
		{
			pUserinfo = _live.front();
			_live.pop_front();
		}

		if (pUserinfo != nullptr)
		{
			_die.push_back(pUserinfo);
			{
				char s[512];
				sprintf(s, "---------->remove: [%d]", pUserinfo->user.id);
				LOGD(s);
			}
		}
	}
}

void UserManager::restoreCache()
{
	{
		std::lock_guard < std::mutex > l(_m);

		std::list<UserSession*>::iterator iter = _die.begin();
		for (; iter != _die.end(); ++iter)
		{
			UserSession* pUserinfo = (*iter);
			delete pUserinfo;
		}

		{			
			std::list<UserSession*>().swap(_die);
		}
	}
}

