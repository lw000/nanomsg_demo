#include "NetMessage.h"

#include <time.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

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

	NetMessage::NetMessage() : message(NULL), messageSize(0), Status(SocketStatus_UNKNOW)
	{
		//::memset(message, 0x0, sizeof(message));
		::memset(&messageHead, 0x0, sizeof(message));
	}

	NetMessage::~NetMessage()
	{
        free(message);
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
		
        message = (lw_char8*)malloc(messageSize*sizeof(lw_char8));
        
		memcpy(message, msg, messageSize);

		ullKey = messageHead.cmd;

		this->Status = Status;
	}
}
