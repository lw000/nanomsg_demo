#include "CacheQueue.h"


CacheQueue::CacheQueue(void) {}
CacheQueue::~CacheQueue(void) {}

int CacheQueue::push(lw_char8* buf, lw_int32 size)
{
	if (size <= 0) return 0;
	lw_int32 i = 0;
	char* p = buf;
	while (i++ < size)
	{
		_buff.push_back(*p++);
	}
	return size;
}

void CacheQueue::pop(lw_int32 position)
{
	if (position <= 0) return;
	if (_buff.empty()) return;
	if (position > _buff.size()) return;

	std::vector<char>::iterator iter = _buff.erase(_buff.begin(), _buff.begin() + position);
}

size_t CacheQueue::size() const
{
	int l = 0;
	{
		l = _buff.size();
	}
	return l;
}

lw_char8* CacheQueue::front()
{
	return _buff.data();
}

void CacheQueue::clear()
{
	_buff.clear();
}