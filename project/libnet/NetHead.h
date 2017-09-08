#ifndef __NetHead_H_
#define __NetHead_H_

#include "common_type.h"
#include <string>

namespace lwstar 
{
	

#pragma  pack(1)
	typedef struct tagNetHead
	{		
		friend std::ostream& operator<<(std::ostream & os, tagNetHead & o);

	public:
		lw_int32	size;			// 数据包大小
		lw_ushort16	v;				// 通讯版本
		lw_int32	cmd;			// 指令
		lw_uint32	ctime;			// 发送时间

	public:
		tagNetHead();

	public:
		std::string debug();

	private:
		

	}  NetHead;

#pragma pack()
}

#endif // !__NetHead_H_
