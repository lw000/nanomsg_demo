#ifndef __buniness_H__
#define __buniness_H__

#include "base_type.h"
#include "common_marco.h"
#include "NetMessage.h"

extern "C" 
{
	typedef std::function<bool(lw_int32 cmd, char* buf, lw_int32 bufsize)> SocketCallback;

	#define SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__)

	typedef void(*LW_PARSE_DATA_CALLFUNC)(lw_int32 cmd, lw_char8* buf, lw_int32 bufsize, lw_void* userdata);
	
	struct LW_NET_MESSAGE
	{
		lw_char8* buf;
		lw_int32 size;
	};

	int lw_socket_init();
	void lw_socket_clean();

	lw_int32 lw_send_socket_data(lw_int32 cmd, void* object, lw_int32 objectSize, std::function<lw_int32(LW_NET_MESSAGE* p)> func);
	lw_int32 lw_parse_socket_data(const lw_char8 * buf, lw_int32 size, LW_PARSE_DATA_CALLFUNC func, lw_void* userdata);

	LW_NET_MESSAGE* lw_create_net_message(lw_int32 cmd, lw_void* object, lw_int32 objectSize);
	lw_void lw_free_net_message(LW_NET_MESSAGE* p);

	class SocketInit
	{
	public:
		SocketInit();
		~SocketInit();
	};
}

#endif // !__buniness_H__

