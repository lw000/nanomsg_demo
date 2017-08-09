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
	typedef typename std::list<T*> LMT;
	typedef typename std::list<T*>::iterator iterator;
	typedef typename std::list<T*>::const_iterator const_iterator;

private:
	std::mutex _m;

private:
	LMT _live;

public:
	ManagerT()
	{

	}

	~ManagerT()
	{
		{
			std::lock_guard < std::mutex > l(_m);
			LMT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				T* t = (*iter);
				delete t;
			}
		}
	}

public:
	T* add(const T* t)
	{
		T* pT = nullptr;
		{
			std::lock_guard < std::mutex > l(_m);
			LMT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				if (t == *iter)
				{
					pT = *iter; break;
				}
			}

			if (pT == nullptr)
			{
				_live.push_back(const_cast<T*>(t));
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
			LMT::iterator iter = _live.begin();
			for (; iter != _live.end(); ++iter)
			{
				if (t == (*iter))
				{
					pT = (*iter);
					
					delete pT;
					pT = nullptr;

					_live.erase(iter);

					break;
				}
			}
		}
	}

	T* find(std::function<bool(T* t)> cond)
	{
		T* pT = nullptr;
		{
			std::lock_guard < std::mutex > l(_m);
			LMT::iterator iter = _live.begin();
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

	T* operator[](int i)
	{
		if ((i < 0) || (i > _live.size()))
		{
			return nullptr;
		}

		T* pT = nullptr;
		int j = 0;
		iterator iter = _live.begin();
		while (iter != _live.end())
		{
			if (j == i)
			{
				pT = (*iter);
				break;
			}
			++iter;
		}

		return pT;
	}
};

#endif	// !__SessionManager_h__
