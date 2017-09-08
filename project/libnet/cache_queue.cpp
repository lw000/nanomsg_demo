#include "cache_queue.h"
#include <algorithm>

namespace lwstar {

	CacheQueue::CacheQueue(void) {
	
	}
	
	CacheQueue::~CacheQueue(void) {
	
	}

	int CacheQueue::push(lw_char8* buf, lw_int32 size)
	{
		if (buf == NULL) return -1;
		if (size <= 0) return -2;
		int i = 0;
		lw_char8* p = buf;
		while (i++ < size) {
			_cq.push_back(*p++);
		}
		return size;
	}

	void CacheQueue::pop(lw_int32 size)
	{
		if (size <= 0) return;
		if (_cq.empty()) return;
		if (size > _cq.size()) return;
		
		std::vector<lw_char8>::iterator iter = _cq.erase(_cq.begin(), _cq.begin() + size);
	}

	void CacheQueue::front_copyto(char* buffer, int size) {
		if (_cq.size() < size) { 
			return;
		}

		char* p = buffer;
		int i = 0;
		for (auto v : _cq) {
			if (i++ >= size) { break; }
			*p++ = v;
		}
	}

	std::vector<lw_char8>* CacheQueue::front_copyto(std::vector<lw_char8>& dest, int size) {
		if (_cq.size() < size)
		{
			return nullptr;
		}

		std::copy_n(_cq.begin(), size, dest.begin());

		return &dest;
	}

	size_t CacheQueue::size() const
	{
		int l = 0;
		{
			l = _cq.size();
		}
		return l;
	}

	lw_char8* CacheQueue::front()
	{
		return _cq.data();
	}

	void CacheQueue::clear()
	{
		_cq.clear();
	}
}
