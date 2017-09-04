#include "NetHead.h"

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#define  TRANSPORT_PROTOCOL_VERSION		10000

namespace lwstar
{
	tagNetHead::tagNetHead()
	{
		this->size = 0;			// 数据包大小
		this->cmd = 0;			// 指令
		this->ctime = 0;	// 发送时间
		this->v =  TRANSPORT_PROTOCOL_VERSION;
	}

	void tagNetHead::debug()
	{
# if defined(_DEBUG) || defined(DEBUG)
		char hhmmss[9];  /* HH:MM:SS\0 */
		time_t t;
		t = (time_t)/*ntohl*/(ctime);
		strftime(hhmmss, sizeof(hhmmss), "%T", localtime(&t));
		printf("size = %d, cmd = %d, create_time = %s \n", size, cmd, hhmmss); 
# endif
	}
}
