#ifndef __SocketMessage_H__
#define __SocketMessage_H__

#include <functional>
#include <string>

#include "base_type.h"
#include "NetHead.h"
#include "common_marco.h"

namespace LW 
{
	class NetMessage;

	typedef std::function<bool(NetMessage* msg)> SEL_NetMessage;

	#define HN_SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, ##__VA_ARGS__)


	class NetMessage 
	{
		public:
			static NetMessage* createMessage();
			static void releaseMessage(NetMessage* message);

		public:
			void setContent(const NetHead* head, lw_char8* msg, lw_int32 msgsize, emSocketStatus Status);

		private:
			NetMessage();
			~NetMessage();

// 		private:
// 			void *operator new(std::size_t ObjectSize);
// 			void operator delete(void *ptrObject);

		public:
			NetHead messageHead;
			lw_uint32 messageSize;			// message size
			lw_char8 message[1024*8];
			lw_ullong64 ullKey;

	private:
		emSocketStatus Status;
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
