#ifndef __Message_H_
#define __Message_H_

#include "base_type.h"

namespace LW 
{
#pragma  pack(1)

	typedef struct tag_sc_userinfo
	{
		lw_int32 id;
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
	} sc_userinfo;

#pragma pack()
}

#endif // !__MessageHead_H_
