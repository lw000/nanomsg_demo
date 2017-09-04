#include "socket_core.h"
#include <assert.h>
#include <string.h>
#include "NetHead.h"
#include "NetMessage.h"
#include "log4z.h"

static const lw_int32 C_NETHEAD_SIZE = sizeof(NetHead);

//////////////////////////////////////////////////////////////////////////////////////////

SocketCore::SocketCore() {

}

SocketCore::~SocketCore() {

}

lw_int32 SocketCore::send(lw_int32 cmd, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func) {
	
	lw_int32 c = -1;
	{
		auto_release_net_message msg(NetMessage::create(cmd, object, objectSize));
		LW_NET_MESSAGE netMsg;
		netMsg.cmd = cmd;
		netMsg.buf = msg->getBuf();
		netMsg.size = msg->getSize();
		c = func(&netMsg);
	}
	return c;
}

lw_int32 SocketCore::parse(const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata)
{
	if (size <= 0) return -1;
	if (NULL == buf) return -2;
	clock_t t = clock();
	{
		{
			std::lock_guard < std::mutex > lock(_m);
			_cq.push(const_cast<lw_char8*>(buf), size);
		}

		lw_int32 dqs = (lw_int32)_cq.size();
		if (dqs >= C_NETHEAD_SIZE) {
			do
			{
				// 解析网络数据包
				{
					std::lock_guard < std::mutex > lock(_m);

					NetHead *phead = (NetHead*)_cq.front();
					if (nullptr == phead) break;

					if (phead->size > dqs) {
						{
							lw_char8 buf[256];
							sprintf(buf, "not a complete data packet [dqs: %d, head size: %d]", dqs, phead->size);
							LOGD(buf);
						}
						break;
					}

					lw_char8* buffer = (lw_char8*)(_cq.front() + C_NETHEAD_SIZE);
					lw_int32 buffer_length = phead->size - C_NETHEAD_SIZE;
					NetMessage* msg = NetMessage::create(phead);
					if (nullptr != msg) {
						msg->setMessage(buffer, buffer_length);
						func(msg->getHead()->cmd, msg->getBuf(), msg->getSize(), userdata);
						NetMessage::release(msg);
					}

					_cq.pop(phead->size);
				}
				dqs = (lw_uint32)_cq.size();
			} while (dqs >= C_NETHEAD_SIZE);
		}
		else {
			lw_char8 buf[256];
			sprintf(buf, "not a complete data packet [dqs: %d, head size: %d]", dqs, C_NETHEAD_SIZE);
			LOGD(buf);
		}
	}
	
	clock_t t1 = clock();
	{
		char s[512];
		sprintf(s, "SocketCore::parse time [%f]", ((double)t1 - t) / CLOCKS_PER_SEC);
		LOGD(s);
	}

	return 0;
}
