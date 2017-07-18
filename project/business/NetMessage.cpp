#include "NetMessage.h"

#include "common_marco.h"

#include <time.h>
#include <stdlib.h>
#include <mutex>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif
namespace LW
{
	static const lw_int32 C_NET_HEAD_SIZE = sizeof(NetHead);

	//////////////////////////////////////////////////////////////////////////

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
	
	NetMessage::NetMessage(lw_int32 cmd, lw_void* msg, lw_int32 size) : NetMessage()
	{
		this->setMessage(cmd, msg, size);
	}

	NetMessage::NetMessage(const NetHead* head) : NetMessage()
	{
		this->setHead(head);
	}

	NetMessage::NetMessage() : _buffsize(0)
	{
		_buff = NULL;
	}

	NetMessage::~NetMessage()
	{
		free(_buff);
	}

#ifdef LW_ENABLE_POOL_
	void *NetMessage::operator new(std::size_t size)
	{
		{
			std::lock_guard < std::mutex > lock(__g_pool_mutex);
			return __g_pool.allocate();
		}
	}

	void NetMessage::operator delete(void *ptr)
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
		t = (time_t)ntohl(_msgHead.createtime);
		_msgHead.createtime = t;
	}

	lw_void NetMessage::setMessage(lw_char8* msg, lw_int32 size)
	{
		if (msg == NULL) return;
		if (size <= 0) return;

		this->_buffsize = size;
		_buff = (lw_char8*)malloc(size * sizeof(lw_char8));
		memcpy(this->_buff, msg, size);
	}

	lw_int32 NetMessage::setMessage(lw_int32 command, lw_void* msg, lw_int32 size)
	{
		if (size < 0) return -1;

		lw_int32 ret = 0;

		do
		{
			_buffsize = C_NET_HEAD_SIZE + size;

			_msgHead.size = _buffsize;
			_msgHead.cmd = command;
			lw_uint32 create_time = (lw_int32)time(NULL);
			_msgHead.createtime = htonl(create_time);

			_buff = (lw_char8*)malloc(_buffsize * sizeof(lw_char8));
			::memcpy(_buff, &_msgHead, sizeof(NetHead));

			if (nullptr != msg && size > 0)
			{
				::memcpy(_buff + sizeof(NetHead), (void*)msg, size);
			}

		} while (0);

		return ret;
	}

	std::string NetMessage::debug()
	{
		lw_char8 buf[256] = { 0 };
		sprintf(buf, "NetHead = {size = %6d cmd = %6d objectSize = %6d}",
			_msgHead.size, _msgHead.cmd, _buffsize);
		return std::string(buf);
	}

	char* NetMessage::getBuff() const
	{
		return _buff;
	}

	lw_int32 NetMessage::getBuffSize()
	{
		return _buffsize;
	}

	const NetHead* NetMessage::getHead()
	{
		return &_msgHead;
	}
}
