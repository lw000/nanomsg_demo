#ifndef __buniness_H__
#define __buniness_H__

#include "base_type.h"

extern "C" 
{
	typedef void(*LW_RECV_SOCKET_CALLFUNC)(void* buf);

	struct LW_NET_MESSAGE
	{
		lw_char8* buf;
		lw_int32 size;
	};

	LW_NET_MESSAGE* lw_create_net_message(lw_int32 cmd, void* object, lw_int32 objectSize);
	void lw_free_net_message(LW_NET_MESSAGE* p);

	lw_int32 lw_on_parse_socket_data(const lw_char8 * buf, lw_int32 bufSize, LW_RECV_SOCKET_CALLFUNC func);
}

#endif // !__buniness_H__
