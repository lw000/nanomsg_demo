#ifndef __NetHead_H_
#define __NetHead_H_

#include <time.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#include "base_type.h"

namespace LW 
{
#pragma  pack(1)
	// network packet header
	typedef struct tagNetHead
	{		
		lw_int32						size;				// 数据包大小
		lw_int32						cmd;				// 指令
		lw_uint32						create_time;		// 发送时间
		lw_int32						reserve;			// 保留字段

	public:
		void debug()
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
	}  NetHead;

#pragma pack()
}

#endif // !__NetHead_H_
