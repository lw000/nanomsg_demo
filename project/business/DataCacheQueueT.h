#ifndef __DataCacheQueueT_H__
#define __DataCacheQueueT_H__

#include <vector>
//#include <mutex>

template<typename _Type>
class DataCacheQueueT
{
public:
	DataCacheQueueT(void) { _vtbuffer = new std::vector<_Type>(); }
	~DataCacheQueueT(void) { SAFE_DELETE(_vtbuffer); }

public:
	void push(_Type* _value, lw_int32 size)
	{
		{
			//std::lock_guard < std::mutex > lock(_mutex);
			lw_int32 i = 0;
			_Type* p = _value;
			while (i++ < size)
			{
				_vtbuffer->push_back(*p++);
			}
		}
		
	}

	void pop(lw_int32 pos)
	{
		if (pos <= 0) return;
		if (_vtbuffer->empty()) return;
		if (_vtbuffer->size() < pos) return;

		{
			//std::lock_guard < std::mutex > lock(_mutex);
			_vtbuffer->erase(_vtbuffer->begin(), _vtbuffer->begin() + pos);
		}	
	}

	size_t size()
	{
		int l = 0;
		{
			//std::lock_guard < std::mutex > lock(_mutex);
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
			//std::lock_guard < std::mutex > lock(_mutex);
			_vtbuffer->clear();
		}
	}

private:
	std::vector<_Type>* _vtbuffer;
	//std::mutex	_mutex;
};



#endif // !__DataCacheQueueT_H__

