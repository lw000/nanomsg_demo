#ifndef __CacheQueue_h__
#define __CacheQueue_h__

#include "common_type.h"
#include <vector>
#include <queue>
#include <stddef.h>

namespace lwstar {
	class CacheQueue
	{
	public:
		CacheQueue(void);
		~CacheQueue(void);

	public:
		int push(lw_char8* buf, lw_int32 size);
		void pop(lw_int32 size);

	public:
		size_t size() const;
		lw_char8* front();
		void front_copyto(char* buffer, int size);
		std::vector<lw_char8>* front_copyto(std::vector<lw_char8>& dest, int size);
		void clear();

	private:
		std::vector<lw_char8> _cq;
	};

}


#endif // !__CacheQueue_h__

