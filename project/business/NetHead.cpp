#include "NetHead.h"

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

namespace LW 
{
	tagNetHead::tagNetHead()
	{
		memset(this, 0x00, sizeof(tagNetHead));
	}

	void tagNetHead::debug()
	{
# if defined(_DEBUG) || defined(DEBUG)
		char hhmmss[9];  /* HH:MM:SS\0 */
		time_t t;
		t = (time_t)ntohl(create_time);
		strftime(hhmmss, sizeof(hhmmss), "%T", localtime(&t));
		printf("size = %d, cmd = %d, u_reserve = %d, create_time = %s \n",
			size,
			cmd,
			reserve,
			hhmmss);
# endif
	}
}
