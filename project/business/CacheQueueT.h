#ifndef __CacheQueueT_H__
#define __CacheQueueT_H__

#include <vector>
#include <queue>

template<typename _Type>
class CacheQueueT
{
public:
	CacheQueueT(void) { _vtbuffer = new std::vector<_Type>(); }
	~CacheQueueT(void) { SAFE_DELETE(_vtbuffer); }

public:
	void push(_Type* buf, lw_int32 size)
	{
		{
			lw_int32 i = 0;
			_Type* p = buf;
			while (i++ < size)
			{
				_vtbuffer->push_back(*p++);
			}
		}
		
	}

	void pop(lw_int32 position)
	{
		if (position <= 0) return;
		if (_vtbuffer->empty()) return;
		if (_vtbuffer->size() < position) return;

		{
			_vtbuffer->erase(_vtbuffer->begin(), _vtbuffer->begin() + position);
		}	
	}

	size_t size()
	{
		int l = 0;
		{
			l = _vtbuffer->size();
		}
		return l;
	}

	_Type* front()
	{
		return _vtbuffer->data();
	}

	void clear()
	{
		{
			_vtbuffer->clear();
		}
	}

private:
	std::vector<_Type>* _vtbuffer;
	std::deque<_Type>* _newvtbuffer;
};



#endif // !__CacheQueueT_H__

