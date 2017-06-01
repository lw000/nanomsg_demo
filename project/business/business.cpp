#include "business.h"

#include <assert.h>
#include <deque>
#include <mutex>
#include <string>

#include "base_type.h"
#include "common_marco.h"
#include "CacheQueueT.h"
#include "NetMessage.h"

using namespace LW;

typedef std::deque<NetMessage*>		NetMessageQueue;
typedef CacheQueueT<lw_char8>		CacheQueue;

NetMessageQueue		__g_msg_queue;
std::mutex			__g_data_mutex;

CacheQueue			__g_cache_queue;
std::mutex			__g_cache_mutex;

static const lw_int32 C_NET_HEAD_SIZE = sizeof(NetHead);

lw_int32 lw_parse_socket_data(const lw_char8 * buf, lw_int32 bufSize, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata)
{
	if (bufSize <= 0) return -1;
	if (NULL == buf) return -2;

	{	
		std::lock_guard < std::mutex > lock(__g_cache_mutex);
		__g_cache_queue.push(const_cast<char*>(buf), bufSize);
	}

	lw_int32 data_queue_size = (lw_int32)__g_cache_queue.size();
	if (data_queue_size >= C_NET_HEAD_SIZE)
	{
		
		do
		{
			{
				std::lock_guard < std::mutex > lock(__g_cache_mutex);
				NetHead *phead = (NetHead*)__g_cache_queue.front();

				if (nullptr == phead) break;

				if (phead->size > data_queue_size)
				{
					printf("not a complete data packet [data_queue_size = %d, pHead->size = %d]\n", data_queue_size, phead->size);
					break;
				}

				lw_char8* data = __g_cache_queue.front() + C_NET_HEAD_SIZE;
				lw_int32 data_len = phead->size - C_NET_HEAD_SIZE;

				NetMessage* msg = NetMessage::createNetMessage(phead);
				if (nullptr != msg)
				{
					msg->setMessage(data, data_len, msgStatus_RECV);
					//__g_msg_queue.push_back(smsg);
					{
						func(msg->getHead()->cmd, msg->getBuff(), msg->getBuffSize(), userdata);
					}
					NetMessage::releaseNetMessage(msg);
				}
				__g_cache_queue.pop(phead->size);
			}

			data_queue_size = (lw_uint32)__g_cache_queue.size();

			/*printf("packet [data_queue_size = %d, pHead->size = %d]\n", data_queue_size, pHead->size);*/		
        } while (data_queue_size >= C_NET_HEAD_SIZE);
	}

	return 0;
}

void lw_get_message(std::function<void(NetMessage* smsg)> func)
{
	int queue_size = 0;
	do
	{
		NetMessage* smsg = nullptr;
		{
			std::lock_guard < std::mutex > lock(__g_data_mutex);
			queue_size = __g_msg_queue.size();
			if (queue_size > 0)
			{
				smsg = __g_msg_queue.front();
				__g_msg_queue.pop_front();
			}
		}
		if (nullptr != smsg)
		{
			func(smsg);
			NetMessage::releaseNetMessage(smsg);
		}
	} while (queue_size > 0);
}

lw_int32 lw_send_socket_data(lw_int32 command, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func)
{
	lw_int32 result = 0;
	{
		LW_NET_MESSAGE* p = lw_create_net_message(command, object, objectSize);
		result = func(p);
		lw_free_net_message(p);
	}
	return result;
}

LW_NET_MESSAGE* lw_create_net_message(lw_int32 command, lw_void* object, lw_int32 objectSize)
{
	NetMessage *msg = NetMessage::createNetMessage(command, object, objectSize);

	LW_NET_MESSAGE * p = (LW_NET_MESSAGE*)malloc(sizeof(LW_NET_MESSAGE));
	p->size = msg->getBuffSize();
	p->buf = (lw_char8*)malloc(p->size * sizeof(lw_char8));
	memcpy(p->buf, msg->getBuff(), msg->getBuffSize());

	NetMessage::releaseNetMessage(msg);

	return p;
}

lw_void lw_free_net_message(LW_NET_MESSAGE* p)
{
	free(p->buf);
	free(p);
	p = NULL;
}
