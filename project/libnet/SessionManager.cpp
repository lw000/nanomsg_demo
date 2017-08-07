#include "SessionManager.h"

#include "socket_session.h"

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
	{
		std::list<SocketSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			SocketSession* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}

	{
		std::list<SocketSession*>::iterator iter = _die.begin();
		for (; iter != _die.end(); ++iter)
		{
			SocketSession* pUserinfo = (*iter);
			delete pUserinfo;
		}
	}
}

SocketSession* SessionManager::add(const SocketSession* session)
{
	// 如果用户不存在，创建对象，添加用户
	// 如果用户存在，更新用户信息

	SocketSession* pSession = nullptr;
	{
		std::lock_guard < std::mutex > l(_m);
		std::list<SocketSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (session == *iter)
			{
				pSession = *iter; break;
			}
		}

		if (pSession == NULL)
		{
			{
				// 缓存中取出对象
				if (!_die.empty())
				{
					pSession = _die.front();
					_die.pop_front();
				}
			}

			if (pSession == NULL)
			{
				pSession = new SocketSession();
			}

			*pSession = *session;

			_live.push_back(const_cast<SocketSession*>(pSession));
		}
		else
		{
			*pSession = *session;
		}
	}

	return pSession;
}

void SessionManager::remove(const SocketSession* session)
{
	{
		std::lock_guard < std::mutex > l(_m);
		SocketSession* pSession = nullptr;
		std::list<SocketSession*>::iterator iter = _live.begin();
		for (; iter != _live.end(); ++iter)
		{
			if (pSession == (*iter))
			{
				pSession = *iter;
				_live.erase(iter);
				break;
			}
		}

		if (pSession != nullptr)
		{
			_die.push_back(pSession);
		}
	}
}

void SessionManager::restoreCache()
{
	{
		std::lock_guard < std::mutex > l(_m);

		{
			std::list<SocketSession*>::iterator iter = _die.begin();
			for (; iter != _die.end(); ++iter)
			{
				SocketSession* p = (*iter);
				delete p;
			}

			std::list<SocketSession*>().swap(_die);
		}
	}
}

