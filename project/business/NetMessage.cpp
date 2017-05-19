#include "NetMessage.h"
#include <winsock.h>
#include <time.h>

namespace LW 
{
	//////////////////////////////////////////////////////////////////////////

	NetMessage* NetMessage::createMessage()
	{
		return new NetMessage();
	}

	void NetMessage::releaseMessage(NetMessage* message)
	{
		SAFE_DELETE(message);
	}

	NetMessage::NetMessage() : messageSize(0), Status(SocketStatus_UNKNOW)
	{
		::memset(message, 0x0, sizeof(message));
		::memset(&messageHead, 0x0, sizeof(message));
	}

	NetMessage::~NetMessage()
	{

	}

// 	void *HNSocketMessage::operator new(std::size_t ObjectSize)
// 	{
// 		return gMemPool.get();
// 	}
// 
// 	void HNSocketMessage::operator delete(void *ptrObject)
// 	{
// 		gMemPool.release(ptrObject);
// 	}

	void NetMessage::setContent(const NetHead* head, lw_char8* msg, lw_int32 msgsize, emSocketStatus Status)
	{
		time_t t;
		t = (time_t)ntohl(head->create_time);

		messageHead = *head;

		messageHead.create_time = t;

		messageSize = msgsize;
		
		memcpy(message, msg, messageSize);

		ullKey = messageHead.cmd;

		this->Status = Status;
	}
}
