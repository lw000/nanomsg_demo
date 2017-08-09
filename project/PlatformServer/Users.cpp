#include "Users.h"
#include <algorithm>

#include "..\libcrossLog\log4z.h"

#include "socket_session.h"

using namespace zsummer::log4z;

Users::Users()
{
}

Users::~Users()
{
	{
		iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			UserSession* pUsession = (*iter);
			{
				delete pUsession->session;
				pUsession->session = nullptr;
			}
			delete pUsession;
		}
	}

	{
		iterator iter = _cache.begin();
		for (; iter != _cache.end(); ++iter)
		{
			UserSession* pUsession = (*iter);
			{
				delete pUsession->session;
				pUsession->session = nullptr;
			}
			delete pUsession;
		}
	}
}

UserSession* Users::find(int uid)
{
	UserSession* pUsession = nullptr;
	{
		std::lock_guard < std::mutex > l(_m);
		iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (uid == (*iter)->user.uid)
			{
				pUsession = *iter;
				break;
			}
		}
	}
	
	return pUsession;
}

UserSession* Users::find(const USER_INFO* user)
{
	if (user != nullptr)
	{
		return this->find(user->uid);
	}

	return nullptr;
}

UserSession* Users::find(const SocketSession* session)
{
	UserSession* pUsession = nullptr;
	{
		std::lock_guard < std::mutex > l(_m);
		iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (session == (*iter)->session)
			{
				pUsession = *iter;
				break;
			}
		}
	}

	return pUsession;
}

int Users::add(const USER_INFO& user, SocketSession* session)
{
	// 如果用户不存在，创建对象，添加用户
	// 如果用户存在，更新用户信息
	int r = 0;

// 	clock_t t = clock();
// 	int uid = 0;
	{
		UserSession* pUsession = nullptr;

		{
			std::lock_guard < std::mutex > l(_m);
			iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				if (user.uid == (*iter)->user.uid)
				{
					pUsession = *iter; break;
				}
			}

			if (pUsession == nullptr)
			{
				// 缓存中取出对象
				if (!_cache.empty())
				{
					pUsession = _cache.front();
					_cache.pop_front();
				}

				if (pUsession == nullptr)
				{
					pUsession = new UserSession;
				}
				_live.push_back(pUsession);
			}

			pUsession->session = session;
			pUsession->user = user;
/*			uid = pUsession->user.uid;*/
		}
	}

// 	clock_t t1 = clock();
// 	{
// 		char s[512];
// 		sprintf(s, "add uid = [%d], time [%f] \n", uid, ((double)t1 - t) / CLOCKS_PER_SEC);
// 		LOGD(s);
// 	}
	
	return r;
}

void Users::remove(const SocketSession* session)
{
	clock_t t = clock();
	int uid = 0;
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* pUsession = nullptr;
		iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (session == (*iter)->session)
			{
				pUsession = *iter;
				_live.erase(iter);
				
				break;
			}
		}

		if (pUsession != nullptr)
		{
			uid = pUsession->user.uid;

			pUsession->reset();
			_cache.push_back(pUsession);
		}
	}
	clock_t t1 = clock();
	{
		char s[512];
		sprintf(s, "remove uid = [%d], time [%f]", uid, ((double)t1 - t) / CLOCKS_PER_SEC);
		LOGD(s);
	}
}

void Users::remove(int uid)
{
	clock_t t = clock();
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* pUsession = nullptr;
		iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (uid == (*iter)->user.uid)
			{
				pUsession = *iter;
				_live.erase(iter);
				break;
			}
		}

		if (pUsession != nullptr)
		{
			pUsession->reset();
			_cache.push_back(pUsession);
		}
	}
	clock_t t1 = clock();
	{
		char s[512];
		sprintf(s, "remove uid = [%d], time [%f]", uid, ((double)t1 - t) / CLOCKS_PER_SEC);
		LOGD(s);
	}
}

void Users::remove(const USER_INFO* user)
{
	if (user != nullptr)
	{
		return this->remove(user->uid);
	}
}

UserSession* Users::operator[](int i)
{
	if ((i < 0) || (i > _live.size()))
	{
		return nullptr;
	}

	UserSession* pUsession = nullptr;
	int j = 0;
	iterator iter = _live.begin();
	while (iter != _live.end())
	{
		if (j == i)
		{
			pUsession = *iter;
			break;
		}
		++iter;
	}

	return pUsession;
}

void Users::removeUserTest()
{
	clock_t t = clock();
	int uid = 0;
	{
		std::lock_guard < std::mutex > l(_m);
		UserSession* pUsession = nullptr;

		// 缓存中取出对象
		if (!_live.empty())
		{
			pUsession = _live.front();
			_live.pop_front();
		}

		if (pUsession != nullptr)
		{
			uid = pUsession->user.uid;

			pUsession->reset();
			_cache.push_back(pUsession);
		}
	}
	clock_t t1 = clock();
	{
		char s[512];
		sprintf(s, "remove uid = [%d], time [%f]", uid, ((double)t1 - t) / CLOCKS_PER_SEC);
		LOGD(s);
	}
}

void Users::restoreCache()
{
	{
		std::lock_guard < std::mutex > l(_m);

		iterator iter = _cache.begin();
		for (; iter != _cache.end(); ++iter)
		{
			UserSession* pUsession = (*iter);
			{
				delete pUsession->session;
				pUsession->session = nullptr;
			}
			delete pUsession;
		}

		{			
			LIST_U().swap(_cache);
		}
	}
}

