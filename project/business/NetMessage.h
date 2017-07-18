#ifndef __SocketMessage_H__
#define __SocketMessage_H__

#include <functional>

#include "NetHead.h"
#include "base_type.h"
#include <iosfwd>
#include <iostream>
#include <string>

namespace LW
{
	#define SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__)

	class NetMessage;

	
	class NetMessage 
	{
	public:
		static NetMessage* createNetMessage(lw_int32 cmd, lw_void* msg, lw_int32 msgsize);
		static NetMessage* createNetMessage(const NetHead* head);

		static lw_void releaseNetMessage(NetMessage* message);

	public:
		lw_void setMessage(lw_char8* msg, lw_int32 size);
		lw_int32 setMessage(lw_int32 cmd, lw_void* msg, lw_int32 size);

	public:
		lw_char8* getBuff() const;
		lw_int32 getBuffSize();
		const NetHead* getHead();

		std::string debug();

	private:
		NetMessage(lw_int32 cmd, lw_void* msg = nullptr, lw_int32 size = 0);
		NetMessage(const NetHead* head);
		NetMessage();
		~NetMessage();

	private:
		lw_void setHead(const NetHead* head);

#ifdef LW_ENABLE_POOL_
	private:
		void *operator new(std::size_t size);
		void operator delete(void *ptr);
#endif

	private:
		NetHead _msgHead;
		lw_uint32 _buffsize;
		lw_char8 *_buff;
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
				NetMessage::releaseNetMessage(_msg);
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

#endif	//__SocketMessage_H__
