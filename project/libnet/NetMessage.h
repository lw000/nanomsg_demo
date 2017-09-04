#ifndef __NetMessage_h__
#define __NetMessage_h__

#include <functional>

#include "NetHead.h"
#include "common_type.h"
#include <iosfwd>
#include <iostream>
#include <string>

namespace lwstar
{
	class NetMessage;

	class NetMessage
	{
	public:
		static NetMessage* create(lw_int32 cmd, lw_void* msg, lw_int32 size);
		static NetMessage* create(const NetHead* head);
		static lw_void release(NetMessage* message);

	public:
		lw_void setMessage(lw_char8* msg, lw_int32 size);
		lw_int32 setMessage(lw_int32 cmd, lw_void* msg, lw_int32 size);

	public:
		lw_char8* getBuf() const;
		lw_int32 getSize() const;
		const NetHead* getHead() const;

	public:
		std::string debug();

	private:
		NetMessage();
		NetMessage(const NetHead* head);
		NetMessage(lw_int32 cmd, lw_void* msg = nullptr, lw_int32 size = 0);
		
	private:
		~NetMessage();

	private:
		NetHead _head;
		lw_char8 *_buf;
		lw_uint32 _size;
	};

	class auto_release_net_message
	{
	public:
		auto_release_net_message(NetMessage *msg)
		{
			_msg = msg;
		}

		~auto_release_net_message()
		{
			if (_msg != NULL)
			{
				NetMessage::release(_msg);
			}
		}

		NetMessage* operator ->()
		{
			return _msg;
		}

	private:
		NetMessage *_msg;
	};
};

#endif	//__NetMessage_H__
