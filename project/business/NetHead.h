#ifndef __NetHead_H_
#define __NetHead_H_

#include "base_type.h"

namespace LW 
{
#pragma  pack(1)
	typedef struct tagNetHead
	{		
	public:
		lw_int32	size;			// 数据包大小
		lw_int32	cmd;			// 指令
		lw_uint32	create_time;	// 发送时间
		lw_int32	reserve;		// 保留字段

	public:
		tagNetHead();

	public:
		void debug();

	private:
		lw_int32	v;				// 通讯版本

	}  NetHead;

#pragma pack()
}

#endif // !__NetHead_H_
