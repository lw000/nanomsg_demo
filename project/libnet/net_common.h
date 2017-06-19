#ifndef __net_common_H__
#define __net_common_H__

#include "base_type.h"
#include <string>

struct CLIENT
{
	struct bufferevent* bev;
	bool connected;
	lw_int32 heartBeat__count;
	std::string _addr;
	std::string _port;

public:
	CLIENT() : bev(NULL), connected(false), heartBeat__count(0)
	{
	}
};


#endif // !__net_common_H__