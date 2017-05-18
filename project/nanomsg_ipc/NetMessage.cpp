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

	NetMessage::NetMessage() : objectSize(0), Status(SocketStatus_UNKNOW)
	{
		::memset(object, 0x0, sizeof(object));
		::memset(&messageHead, 0x0, sizeof(object));
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

	void NetMessage::setContent(const NetHead* head, lw_char8* obj, lw_int32 Size)
	{
		time_t t;
		t = (time_t)ntohl(head->u_send_time);

		messageHead = *head;

		messageHead.u_send_time = t;

		objectSize = Size;
		memcpy(object, obj, objectSize);
		ullKey = messageHead.ull_cmd;
	}
}
