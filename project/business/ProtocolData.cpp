#include "ProtocolData.h"
#include <assert.h>
#include <string.h>

namespace LW 
{
	ProtocolData::ProtocolData() : _bufferSize(0), _buffer(NULL)
	{
		//::memset(_object, 0x0, sizeof(_object));
		::memset(&_messageHead, 0x0, sizeof(NetHead));
	}

	ProtocolData::~ProtocolData()
	{
		free(_buffer);
	}

	lw_int32 ProtocolData::createPackage(lw_int32 cmd, void* object, lw_int32 objectSize)
	{
		lw_int32 ret = 0;

		do
		{
			if (objectSize <= 0)
			{
				ret = -1;
				break;
			}

			_bufferSize = sizeof(NetHead) + objectSize;

			_messageHead.size = _bufferSize;
			_messageHead.cmd = cmd;
			lw_uint32 create_time = (lw_int32)time(NULL);
			_messageHead.create_time = htonl(create_time);

			_buffer = (lw_char8 *)malloc(_bufferSize * sizeof(lw_char8));

			::memcpy(_buffer, &_messageHead, sizeof(NetHead));

			if (nullptr != object)
			{
				::memcpy(_buffer + sizeof(NetHead), (void*)object, objectSize);
			}

			debug();
		} while (0);

		return ret;
	}

	lw_char8* ProtocolData::getContent()
	{
		return _buffer;
	}

	lw_int32 ProtocolData::getContentSize() const
	{
		return _bufferSize;
	}

	void ProtocolData::debug()
	{
# if defined(_DEBUG) || defined(DEBUG)
		lw_char8 buf[256] = { 0 };
		sprintf(buf, "NetHead = {size = %d cmd = %d reserve = %d object = %s objectSize = %d}",
			_messageHead.size,
			_messageHead.cmd,
			_messageHead.reserve,
			_buffer,
			_bufferSize);
#endif	//_DEBUG || DEBUG
	}
}
