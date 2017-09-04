#ifndef __CacheQueueT_H__
#define __CacheQueueT_H__

#include "common_type.h"
#include <vector>

namespace lwstar {
	class CacheQueue
	{
	public:
		CacheQueue(void);
		~CacheQueue(void);

	public:
		int push(lw_char8* buf, lw_int32 size);
		void pop(lw_int32 position);

	public:
		size_t size() const;
		lw_char8* front();
		void clear();

	private:
		std::vector<lw_char8> _buff;
	};

}


#endif // !__CacheQueueT_H__

