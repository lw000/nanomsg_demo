#ifndef __SocketMessage_H__
#define __SocketMessage_H__

#include <functional>

#include "NetHead.h"
#include "base_type.h"

namespace LW 
{
	class NetMessage;

	typedef std::function<bool(NetMessage* msg)> SEL_NetMessage;

	#define SOCKET_CALLBACK(__selector__,__target__, ...) std::bind(&__selector__,__target__, std::placeholders::_1, ##__VA_ARGS__)

	class NetMessage 
	{
	public:
		static NetMessage* createNetMessage();
		static NetMessage* createNetMessage(lw_int32 command, lw_void* msg, lw_int32 msgsize);
		static NetMessage* createNetMessage(const NetHead* head);

		static lw_void releaseNetMessage(NetMessage* message);

	public:
		lw_void setMessage(lw_char8* msg, lw_int32 msgsize, enMsgStatus Status);
		lw_int32 setMessage(lw_int32 command, lw_void* msg, lw_int32 msgsize);

	public:
		lw_char8* getBuff() const;
		lw_int32 getBuffSize();
		const NetHead* getHead();

		lw_void debug();

	private:
		NetMessage(lw_int32 cmd, lw_void* msg = nullptr, lw_int32 msgsize = 0);
		NetMessage(const NetHead* head);
		NetMessage();
		~NetMessage();

	private:
		lw_void setHead(const NetHead* head);

#ifdef LW_ENABLE_POOL_
	private:
		void *operator new(std::size_t ObjectSize);
		void operator delete(void *ptrObject);
#endif

	private:
		NetHead _msgHead;
		lw_uint32 _buff_size;
		lw_char8 *_buff;
		enMsgStatus _status;
	};
};

#endif	//__SocketMessage_H__
