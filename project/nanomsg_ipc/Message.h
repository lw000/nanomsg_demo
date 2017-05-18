#ifndef __Message_H_
#define __Message_H_

#include "BaseType.h"

namespace LW 
{
#pragma  pack(1)

	typedef struct tagUserInfo
	{
		lw_int32 age;
		lw_int32 sex;
		lw_char8 name[32];
		lw_char8 address[256];

	public:

		void debug()
		{
# if defined(_DEBUG) || defined(DEBUG)
			printf("sex = %d, age = %d, name = %s address = %s\n", sex, age, name, address);
# endif
		}

	} UserInfo;

#pragma pack()
}

#endif // !__MessageHead_H_
