#include "socket_core.h"

#include <assert.h>
#include <string.h>
#include <memory>
#include <thread>

#include "NetHead.h"
#include "NetPackage.h"
#include <log4z.h>

static const lw_int32 C_NETHEAD_SIZE = sizeof(NetHead);

//////////////////////////////////////////////////////////////////////////////////////////

SocketCore::SocketCore() {

}

SocketCore::~SocketCore() {

}

lw_int32 SocketCore::send(lw_int32 cmd, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func) {
	
	lw_int32 c = -1;
	{
		std::unique_ptr<NetPackage> msg(new NetPackage(cmd, object, objectSize));
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

//	clock_t t = clock();

	{
		{
			lw_int32 dqs = 0;
			{
				std::lock_guard < std::mutex > lock(_m);
				_cq.push(const_cast<lw_char8*>(buf), size);
				dqs = (lw_int32)_cq.size();
			}

			if (C_NETHEAD_SIZE <= dqs) {
				do
				{
					std::unique_ptr<lw_char8[]> package;
					{
						std::lock_guard < std::mutex > lock(_m);

						NetHead *hd = (NetHead*)_cq.front();
						if (dqs < hd->size) {
							lw_char8 buf[256];
							sprintf(buf, "thread::id:%d, not a complete data packet [thread::id: %d, dqs: %d, head size: %d]", std::this_thread::get_id(), dqs, hd->size);
							LOGD(buf);

							break;
						}
						else
						{
							package.reset(new lw_char8[hd->size]);
							_cq.front_copyto(package.get(), hd->size);
							_cq.pop(hd->size);
							dqs = (lw_uint32)_cq.size();
						}
					}
					
					NetHead *phd = (NetHead*)package.get();

					LOGD(phd->debug());

					lw_char8* buf = &package[C_NETHEAD_SIZE];
					lw_int32 buf_len = phd->size - C_NETHEAD_SIZE;

					{
						std::unique_ptr<NetPackage> msg(new NetPackage(phd));
						if (nullptr != msg) {
							msg->setMessage(buf, buf_len);
							func(msg->getHead()->cmd, msg->getBuf(), msg->getSize(), userdata);
						}
					}
				} while (dqs >= C_NETHEAD_SIZE);
			}
			else {
				lw_char8 buf[256];
				sprintf(buf, "not a complete data packet [thread::id: %d, dqs: %d]", std::this_thread::get_id(), dqs);
				LOGD(buf);
			}
		}
	}
	
//	clock_t t1 = clock();
// 	{
// 		char s[512];
// 		sprintf(s, "SocketCore::parse time [%f]", ((double)t1 - t) / CLOCKS_PER_SEC);
// 		LOGD(s);
// 	}

	return 0;
}
