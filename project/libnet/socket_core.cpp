#include "socket_core.h"

#include <assert.h>
#include <deque>
#include <mutex>
#include <string>
#include <memory>

#include "common_type.h"

using namespace LW;

static const lw_int32 C_NET_HEAD_SIZE = sizeof(NetHead);

static lw_int32 __parse_socket_data(CacheQueue& cache, std::mutex& m, const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata);

static LW_NET_MESSAGE* __createMessage(lw_int32 cmd, lw_void* object, lw_int32 objectSize);
static lw_void __freeMessage(LW_NET_MESSAGE* p);

//////////////////////////////////////////////////////////////////////////////////////////

LW_NET_MESSAGE* __createMessage(lw_int32 cmd, lw_void* object, lw_int32 objectSize)
{
	LW_NET_MESSAGE * p = NULL;

	{
		auto_release_net_message msg(NetMessage::create(cmd, object, objectSize));
		p = (LW_NET_MESSAGE*)malloc(sizeof(LW_NET_MESSAGE));
		p->size = msg->getBuffSize();
		p->buf = (lw_char8*)malloc(p->size * sizeof(lw_char8));
		memcpy(p->buf, msg->getBuff(), p->size);
	}

	return p;
}

lw_void __freeMessage(LW_NET_MESSAGE* p)
{
	free(p->buf);
	free(p);
	p = NULL;
}

static lw_int32 __parse_socket_data(CacheQueue& cache, std::mutex& m, const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata)
{
	if (size <= 0) return -1;
	if (NULL == buf) return -2;

	{
		std::lock_guard < std::mutex > lock(m);
		cache.push(const_cast<lw_char8*>(buf), size);
	}

	lw_int32 data_queue_size = (lw_int32)cache.size();
	if (data_queue_size >= C_NET_HEAD_SIZE)
	{
		do
		{
			// 解析网络数据包
			{
				std::lock_guard < std::mutex > lock(m);

				NetHead *phead = (NetHead*)cache.front();
				if (nullptr == phead) break;

				if (phead->size > data_queue_size)
				{
					printf("not a complete data packet [data_queue_size = %d, pHead->size = %d]\n", data_queue_size, phead->size);
					break;
				}

				lw_char8* buffer = (lw_char8*)(cache.front() + C_NET_HEAD_SIZE);
				lw_int32 buffer_length = phead->size - C_NET_HEAD_SIZE;
				NetMessage* msg = NetMessage::create(phead);
				if (nullptr != msg)
				{
					msg->setMessage(buffer, buffer_length);
					{
						func(msg->getHead()->cmd, msg->getBuff(), msg->getBuffSize(), userdata);
					}
					NetMessage::release(msg);
				}
				cache.pop(phead->size);
			}
			data_queue_size = (lw_uint32)cache.size();
		} while (data_queue_size >= C_NET_HEAD_SIZE);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

SocketCore::SocketCore() {

}

SocketCore::~SocketCore() {

}

lw_int32 SocketCore::send(lw_int32 cmd, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func) {
	lw_int32 c = -1;
	{
		LW_NET_MESSAGE* p = __createMessage(cmd, object, objectSize);
		c = func(p);
		__freeMessage(p);
	}
	return c;
}

lw_int32 SocketCore::parse(const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata)
{
	int c = __parse_socket_data(_cacheQueue, _cacheMutex, buf, size, func, userdata);
	return c;
}
