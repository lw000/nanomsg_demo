#ifndef __NetHead_H_
#define __NetHead_H_

#include "BaseType.h"
#include <winsock.h>
#include <time.h>

namespace LW 
{
#pragma  pack(1)
	// network packet header
	typedef struct tagNetHead
	{		
		lw_int32						size;				// 数据包大小
		lw_int32						cmd;				// 指令
		lw_int32						checkcode;			// 校验码
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
			printf("u_size = %u, u_cmd = %ull, u_checkcode = %u, u_reserve = %u, u_send_time = %s \n",
				size,
				cmd,
				checkcode,
				reserve,
				hhmmss);
# endif 
		}
	}  NetHead;

#pragma pack()
}

#endif // !__NetHead_H_
