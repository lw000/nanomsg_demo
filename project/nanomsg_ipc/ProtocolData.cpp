#include "ProtocolData.h"
#include <assert.h>

namespace LW 
{
	ProtocolData::ProtocolData() : _objectSize(0)
	{
		::memset(_object, 0x0, sizeof(_object));
		::memset(&_messageHead, 0x0, sizeof(NetHead));
	}

	lw_int32 ProtocolData::createPackage(lw_int32 cmd, lw_int32 checkcode, void* object, lw_int32 objectSize)
	{
		lw_int32 ret = 0;

		_objectSize = sizeof(NetHead) + objectSize;

		assert(_objectSize < CACHE_BUFFER_SIZE, "data size > cache size.");

		if (_objectSize <= CACHE_BUFFER_SIZE)
		{
			_messageHead.size = _objectSize;
			_messageHead.cmd = cmd;
			_messageHead.checkcode = checkcode;

			lw_uint32 create_time = (lw_int32)time(NULL);
			_messageHead.create_time = htonl(create_time);

			::memcpy(_object, &_messageHead, sizeof(NetHead));
			
			if (nullptr != object)
			{
				::memcpy(_object + sizeof(NetHead), (void*)object, _objectSize);
			}

			debug();

			ret = 0;
		}
		else
		{
			ret = -2;
		}

		return ret;
	}

	lw_char8* ProtocolData::getContent()
	{
		return _object;
	}

	lw_int32 ProtocolData::getContentSize() const
	{
		return _objectSize;
	}

	void ProtocolData::debug()
	{
# if defined(_DEBUG) || defined(DEBUG)
		lw_char8 buf[256] = { 0 };
		sprintf(buf, "NetHead = {size = %d cmd = %d reserve = %d object = %s objectSize = %d}",
			_messageHead.size,
			_messageHead.cmd,
			_messageHead.reserve,
			_object,
			_objectSize);
#endif	//_DEBUG || DEBUG
	}
}
