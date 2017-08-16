#ifndef __buniness_h__
#define __buniness_h__

#include "common_type.h"
#include "common_marco.h"
#include "NetMessage.h"
#include "CacheQueue.h"

#include <mutex>

#define SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__)

typedef void(*LW_PARSE_DATA_CALLFUNC)(lw_int32 cmd, lw_char8* buf, lw_int32 bufsize, lw_void* userdata);

struct LW_NET_MESSAGE
{
	lw_char8* buf;
	lw_int32 size;
};

class SocketCore final
{
public:
	SocketCore();
	~SocketCore();

public:
	lw_int32 send(lw_int32 cmd, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func);
	lw_int32 parse(const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata);

private:
	CacheQueue	_cacheQueue;
	std::mutex	_cacheMutex;
};

#endif // !__buniness_h__

