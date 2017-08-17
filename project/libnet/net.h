#ifndef __net_h__
#define __net_h__

#include "common_type.h"

extern "C"
{
	int lw_socket_init();
	void lw_socket_clean();
}

class SocketInit final
{
public:
	SocketInit();
	~SocketInit();
};

#endif // !__net_h__