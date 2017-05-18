#ifndef __SocketMessage_H__
#define __SocketMessage_H__

#include <functional>
#include <string>

#include "BaseType.h"
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
			void setContent(const NetHead* head, lw_char8* obj, lw_int32 Size);

		private:
			NetMessage();
			~NetMessage();

// 		private:
// 			void *operator new(std::size_t ObjectSize);
// 			void operator delete(void *ptrObject);

		public:
			NetHead messageHead;
			lw_uint32 objectSize;			// message size
			lw_char8 object[4096];
			emSocketStatus Status;
			lw_ullong64 ullKey;
	};

	class SocketSelectorItem
	{
	public:
		SocketSelectorItem(SEL_NetMessage selector) : _selector(selector)
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
