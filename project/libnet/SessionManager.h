#ifndef __SessionManager_h__
#define __SessionManager_h__

#include <list>
#include <mutex>

class SocketSession;

class SessionManager final
{
private:
	std::mutex _m;

private:
	std::list<SocketSession*> _live;
	std::list<SocketSession*> _die;

public:
	SessionManager();
	~SessionManager();

public:
	SocketSession* add(const SocketSession* session);
	void remove(const SocketSession* session);

public:
	void restoreCache();
};

template<class T>
class ManagerT
{
	typedef std::list<T*> LT;

private:
	std::mutex _m;

private:
	LT _live;
	LT _die;

public:
	ManagerT()
	{

	}

	~ManagerT()
	{
		{
			std::lock_guard < std::mutex > l(_m);
			LT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				T* t = (*iter);
				delete t;
			}
		}

		{
			std::lock_guard < std::mutex > l(_m);
			LT::iterator iter = _die.begin();
			for (; iter != _die.end(); ++iter)
			{
				T* t = (*iter);
				delete t;
			}
		}
	}

public:
	T* add(const T* t)
	{
		// 如果用户不存在，创建对象，添加用户
		// 如果用户存在，更新用户信息

		T* pT = nullptr;
		{
			std::lock_guard < std::mutex > l(_m);
			LT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				if (t == *iter)
				{
					pT = *iter; break;
				}
			}

			if (pT == NULL)
			{
				{
					// 缓存中取出对象
					if (!_die.empty())
					{
						pT = _die.front();
						_die.pop_front();
					}
				}

				if (pT == NULL)
				{
					pT = new T();
				}

				*pT = *t;

				_live.push_back(const_cast<T*>(pT));
			}
			else
			{
				*pT = *t;
			}
		}

		return pT;
	}

	void remove(T* t)
	{
		{
			std::lock_guard < std::mutex > l(_m);
			T* pT = nullptr;
			LT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				if (pT == (*iter))
				{
					pT = *iter;
					_live.erase(iter);
					break;
				}
			}

			if (pT != nullptr)
			{
				_die.push_back(pT);
			}
		}
	}

	T* find(std::function<bool(T* t)> cond)
	{
		T* pT = nullptr;
		{
			std::lock_guard < std::mutex > l(_m);
			LT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				pT = *iter;
				bool r = cond(pT);
				if (r)
				{
					break;
				}
			}
		}

		return pT;
	}

public:
	void restoreCache()
	{
		{
			std::lock_guard < std::mutex > l(_m);

			{
				LT::iterator iter = _die.begin();
				for (; iter != _die.end(); ++iter)
				{
					T* p = (*iter);
					delete p;
				}

				LT().swap(_die);
			}
		}
	}

};

#endif	// !__SessionManager_h__
