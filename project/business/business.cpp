#include "business.h"

#include <assert.h>
#include <deque>
#include <mutex>

#include "common_marco.h"

#include "NetHead.h"
#include "NetMessage.h"
#include "ProtocolData.h"
#include "DataCacheQueueT.h"

using namespace LW;

typedef std::deque<NetMessage*>			NetMessageQueue;
typedef DataCacheQueueT<lw_char8>		DataCacheQueue;

NetMessageQueue			__g_msg_queue;

DataCacheQueue			__g_data_queue;

std::mutex				__g_data_mutex;

lw_int32 lw_on_parse_socket_data(const lw_char8 * buf, lw_int32 bufSize, LW_RECV_SOCKET_CALLFUNC func)
{
	if (bufSize <= 0) return -1;
	if (NULL == buf) return -2;

	const lw_int32 NeMsgHeadSize = sizeof(NetHead);

	__g_data_queue.push(const_cast<char*>(buf), bufSize);

	NetHead* pHead = nullptr;

	lw_int32 msgSize = (lw_int32)__g_data_queue.size();
	if (msgSize >= NeMsgHeadSize)
	{
		do
		{
			pHead = (NetHead*)__g_data_queue.front();
			if (nullptr != pHead && msgSize >= pHead->size)
			{
				pHead->debug();

				lw_char8* pData = (__g_data_queue.front() + NeMsgHeadSize);

				NetMessage* smsg = NetMessage::createMessage();			
				if (smsg)
				{
					smsg->setContent(pHead, pData, pHead->size - NeMsgHeadSize, SocketStatus_RECV);

					__g_data_queue.pop(pHead->size);
					
					//分发数据
					//__g_msg_queue.push_back(smsg);

					func(smsg);

					NetMessage::releaseMessage(smsg);
				}
				else
				{
					printf("%s >> not a complete data packet [msgSize = %d, pHead->size = %d]",
						"1111111", msgSize, pHead->size);
				}
				msgSize = (lw_uint32)__g_data_queue.size();
			}
		} while (msgSize >= pHead->size);
	}

	return 0;
}

void lw_get_message(std::function<void(NetMessage* smsg)> func)
{
	int queueSize = 0;
	do
	{
		NetMessage* smsg = nullptr;
		{
			std::lock_guard < std::mutex > lock(__g_data_mutex);
			queueSize = __g_msg_queue.size();
			if (queueSize > 0)
			{
				smsg = __g_msg_queue.front();
				__g_msg_queue.pop_front();
			}
		}
		if (nullptr != smsg)
		{
			func(smsg);
			NetMessage::releaseMessage(smsg);
		}
	} while (queueSize > 0);
	//Director::getInstance()->getScheduler()->pauseTarget(this);
}


LW_NET_MESSAGE* lw_create_net_message(lw_int32 cmd, void* object, lw_int32 objectSize)
{
	ProtocolData protocolData;
	protocolData.createPackage(cmd, object, objectSize);

	LW_NET_MESSAGE * p = (LW_NET_MESSAGE*)malloc(sizeof(LW_NET_MESSAGE));

	p->size = protocolData.getContentSize();

	p->buf = (lw_char8*)malloc(p->size * sizeof(lw_char8));
	memcpy(p->buf, protocolData.getContent(), p->size);
	

	return p;
}

void lw_free_net_message(LW_NET_MESSAGE* p)
{
	free(p->buf);
	free(p);
	p = NULL;
}
