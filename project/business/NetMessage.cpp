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

	NetMessage* NetMessage::createNetMessage(lw_int32 cmd, lw_void* msg, lw_int32 msgsize)
	{
		return new NetMessage(cmd, msg, msgsize);
	}

	NetMessage* NetMessage::createNetMessage(const NetHead* head)
	{
		return new NetMessage(head);
	}

	lw_void NetMessage::releaseNetMessage(NetMessage* message)
	{
		SAFE_DELETE(message);
	}
	
	NetMessage::NetMessage(lw_int32 command, lw_void* msg, lw_int32 msgsize) : NetMessage()
	{
		this->setMessage(command, msg, msgsize);
	}

	NetMessage::NetMessage(const NetHead* head) : NetMessage()
	{
		setHead(head);
	}

	NetMessage::NetMessage() : _buff_size(0), _status(msgStatus_UNKNOW)
	{
#ifdef LW_ENABLE_POOL_
		::memset(&_buff, 0x0, sizeof(_buff));
#else
		_buff = NULL;
#endif	
	}

	NetMessage::~NetMessage()
	{
#if !defined(LW_ENABLE_POOL_)
		free(_buff);
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

	lw_void NetMessage::setHead(const NetHead* head)
	{
		if (head == NULL) return;
		
		_msgHead = *head;

		time_t t;
		t = (time_t)ntohl(_msgHead.create_time);
		_msgHead.create_time = t;
	}

	lw_void NetMessage::setMessage(lw_char8* msg, lw_int32 msgsize, enMsgStatus Status)
	{
		if (msg == NULL) return;
		if (msgsize <= 0) return;

		this->_buff_size = msgsize;

#if !defined(LW_ENABLE_POOL_)
		_buff = (lw_char8*)malloc(msgsize * sizeof(lw_char8));
#endif 
		memcpy(this->_buff, msg, msgsize);

		this->_status = Status;
	}

	lw_int32 NetMessage::setMessage(lw_int32 command, lw_void* msg, lw_int32 msgsize)
	{
		if (msgsize < 0) return -1;

		lw_int32 ret = 0;

		do
		{
			_buff_size = sizeof(NetHead) + msgsize;

			_msgHead.size = _buff_size;
			_msgHead.cmd = command;
			lw_uint32 create_time = (lw_int32)time(NULL);
			_msgHead.create_time = htonl(create_time);

#if !defined(LW_ENABLE_POOL_)
			_buff = (lw_char8*)malloc(_buff_size * sizeof(lw_char8));
#endif 
			::memcpy(_buff, &_msgHead, sizeof(NetHead));

			if (nullptr != msg && msgsize > 0)
			{
				::memcpy(_buff + sizeof(NetHead), (void*)msg, msgsize);
			}

		} while (0);

		return ret;
	}

	lw_void NetMessage::debug()
	{
# if defined(_DEBUG) || defined(DEBUG)
		lw_char8 buf[256] = { 0 };
		sprintf(buf, "NetHead = {size = %d cmd = %d reserve = %d object = %s objectSize = %d}",
			_msgHead.size,
			_msgHead.cmd,
			_msgHead.reserve,
			buf,
			_buff_size);
#endif	//_DEBUG || DEBUG
	}

	char* NetMessage::getBuff() const
	{
		return _buff;
	}

	lw_int32 NetMessage::getBuffSize()
	{
		return _buff_size;
	}

	const NetHead* NetMessage::getHead()
	{
		return &_msgHead;
	}
}
