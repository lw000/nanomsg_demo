#include "ProtocolData.h"
#include <assert.h>

namespace LW 
{
	ProtocolData::ProtocolData() : _objectSize(0)
	{
		::memset(_object, 0x0, sizeof(_object));
		::memset(&_messageHead, 0x0, sizeof(NetHead));
	}

	lw_int32 ProtocolData::createPackage(lw_ullong64 ull_cmd, lw_uint32 checkcode, void* object, lw_int32 objectSize)
	{
		lw_int32 ret = 0;

		assert(INVALID_VALUE != checkcode);
		if (INVALID_VALUE == checkcode) return -1;

		_objectSize = sizeof(NetHead) + objectSize;

		assert(_objectSize < CACHE_BUFFER_SIZE, "data size > cache size.");

		if (_objectSize <= CACHE_BUFFER_SIZE)
		{
			_messageHead.u_size = _objectSize;
			_messageHead.ull_cmd = ull_cmd;
			_messageHead.u_checkcode = checkcode;

			lw_uint32 secs = (lw_int32)time(NULL);
			_messageHead.u_send_time = htonl(secs);

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
		lw_char8 buf[512] = { 0 };
		sprintf(buf, "NetMessageHead = {uMessageSize = %ud u_cmd = %d bReserve = %ud object = %s objectSize = %d}",
			_messageHead.u_size,
			_messageHead.ull_cmd,
			_messageHead.u_reserve,
			_object,
			_objectSize);
#endif	//_DEBUG || DEBUG
	}
}
