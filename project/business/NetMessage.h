#ifndef __SocketMessage_H__
#define __SocketMessage_H__

#include <functional>
#include "NetHead.h"

namespace LW 
{
	class NetMessage;

	typedef std::function<bool(NetMessage* msg)> SEL_NetMessage;

	#define SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, ##__VA_ARGS__)

	class NetMessage 
	{
		public:
			static NetMessage* createNetMessage();
			static void releaseNetMessage(NetMessage* message);

		public:
			void setMessage(const NetHead* head, lw_char8* msg, lw_int32 msgsize, enMsgStatus Status);
		
		private:
			NetMessage();
			~NetMessage();

#ifdef LW_ENABLE_POOL_
	private:
		void *operator new(std::size_t ObjectSize);
		void operator delete(void *ptrObject);
#endif

		public:
			NetHead messageHead;
			lw_uint32 messageSize;// message size

#ifdef LW_ENABLE_POOL_
			lw_char8 message[8 * 1024];
#else
			lw_char8 *message;
#endif        
			lw_ullong64 ullKey;

	private:
		enMsgStatus Status;
	};

	class NetMessageSelectorItem
	{
	public:
		NetMessageSelectorItem(SEL_NetMessage selector) : _selector(selector)
		{
		}

	public:
		bool doCallSelector(NetMessage* socketMessage)
		{ 
			return (_selector)(socketMessage);
		}

	private:
		SEL_NetMessage	_selector;
	};
};

#endif	//__SocketMessage_H__
