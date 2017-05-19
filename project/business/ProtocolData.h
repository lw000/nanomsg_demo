#ifndef __ProtocolData_H__
#define __ProtocolData_H__

#include <stdio.h>

#include "base_type.h"
#include "NetHead.h"

namespace LW
{
	class ProtocolData
	{
	public:
		ProtocolData();
		~ProtocolData();

	public:
		lw_int32 createPackage(lw_int32 cmd, void* object = nullptr, lw_int32 objectSize = 0);
	
	public:
		lw_char8* getContent();
		lw_int32 getContentSize() const;

	public:
		void debug();

	private:
		NetHead		_messageHead;
		lw_char8	*_buffer;
		lw_int32	_bufferSize;
	};
}

#endif	//__ProtocolData_H__

