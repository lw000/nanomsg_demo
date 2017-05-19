#ifndef __ProtocolData_H__
#define __ProtocolData_H__

#include <stdio.h>

#include "BaseType.h"
#include "NetHead.h"

namespace LW
{
	class ProtocolData
	{
	public:
		ProtocolData();

	public:
		lw_int32 createPackage(lw_int32 cmd, lw_int32 checkcode, void* object = nullptr, lw_int32 objectSize = 0);
	
	public:
		lw_char8* getContent();
		lw_int32 getContentSize() const;

	public:
		void debug();

	public:
		const static lw_uint32 CACHE_BUFFER_SIZE = 1024*10;

	private:
		NetHead		_messageHead;
		lw_char8		_object[CACHE_BUFFER_SIZE];
		lw_int32		_objectSize;
	};
}

#endif	//__ProtocolData_H__

