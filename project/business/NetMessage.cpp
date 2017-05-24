#include "NetMessage.h"

#include <time.h>
#include <stdlib.h>
#include <mutex>

#include "common_marco.h"

#include "MemoryPool.h"

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

namespace LW
{
	//////////////////////////////////////////////////////////////////////////
#ifdef LW_ENABLE_POOL_
	std::mutex										__g_pool_mutex;
	MemoryPool<NetMessage, sizeof(NetMessage) * 2>	__g_pool;
#endif // WL_ENABLE_POOL_

	NetMessage* NetMessage::createNetMessage()
	{
		return new NetMessage();
	}

	void NetMessage::releaseNetMessage(NetMessage* message)
	{
		SAFE_DELETE(message);
	}

	NetMessage::NetMessage() : messageSize(0), Status(msgStatus_UNKNOW)
	{
#ifdef LW_ENABLE_POOL_
		::memset(&message, 0x0, sizeof(message));
#else
		message = NULL;
#endif	
	}

	NetMessage::~NetMessage()
	{
#if !defined(LW_ENABLE_POOL_)
		free(message);
#endif
	}

#ifdef LW_ENABLE_POOL_
	void *NetMessage::operator new(std::size_t ObjectSize)
	{
		{
			std::lock_guard < std::mutex > lock(__g_pool_mutex);
			return __g_pool.allocate();
		}
	}

	void NetMessage::operator delete(void *ptrObject)
	{
		{
			std::lock_guard < std::mutex > lock(__g_pool_mutex);
			__g_pool.deallocate((NetMessage*)ptrObject);
		}
		
	}
#endif

	void NetMessage::setMessage(const NetHead* head, lw_char8* msg, lw_int32 msgsize, enMsgStatus Status)
	{
		time_t t;
		t = (time_t)ntohl(head->create_time);

		messageHead = *head;

		messageHead.create_time = t;

		messageSize = msgsize;

#if !defined(LW_ENABLE_POOL_)
		message = (lw_char8*)malloc(messageSize * sizeof(lw_char8));
#endif 

		memcpy(message, msg, messageSize);
		ullKey = messageHead.cmd;

		this->Status = Status;
	}

}
