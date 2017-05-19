
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <deque>
#include <mutex>

#include <iostream>

#include <nn.h>
#include <pair.h>

#include <windows.h>

#include "BaseType.h"
#include "common_marco.h"

#include "NetHead.h"
#include "Message.h"
#include "NetMessage.h"
#include "ProtocolData.h"

#include "lm.pb.h"
#include "msg.pb.h"

using namespace LW;

#define NODE0 "node0"
#define NODE1 "node1"

#define SOCKET_ADDR "ipc:///tmp/pair.ipc"

#ifdef _WIN32
#define SLEEP(seconds) SleepEx(seconds * 1000, 1);
#else
#define SLEEP(seconds) sleep(seconds);
#endif

struct transfer_data
{
	int size;
	char data[4096];

public:
	void reset()
	{
		memset(&data, 0x00, sizeof(transfer_data));
	}

	lw_int32 set_data(const char* val, int valsize)
	{
		if (NULL == val) return -1;
		if (valsize < 0) return -2;
		if (valsize > 4096) return -3;
		
		memcpy(data, val, valsize);
		size = valsize;

		return 0;
	}
};


template<typename _Type>
class DataCacheQueueT
{
public:
	DataCacheQueueT(void) { _vtbuffer = new std::vector<_Type>(); }
	~DataCacheQueueT(void) { SAFE_DELETE(_vtbuffer); }

public:
	void push(_Type* _value, lw_int32 size)
	{
		lw_int32 i = 0;
		_Type* p = _value;
		while (i++ < size)
		{
			_vtbuffer->push_back(*p++);
		}
	}

	void pop(lw_int32 pos)
	{
		if (pos <= 0) return;
		if (_vtbuffer->empty()) return;
		if (_vtbuffer->size() > pos) return;

		_vtbuffer->erase(_vtbuffer->begin(), _vtbuffer->begin() + pos);
	}

	size_t size()
	{
		return _vtbuffer->size();
	}

	_Type* front()
	{
		return _vtbuffer->data();
	}

	void clear()
	{
		_vtbuffer->clear();
	}

private:
	std::vector<_Type>* _vtbuffer;
};

typedef std::deque<NetMessage*>			NetMessageQueue;
typedef DataCacheQueueT<lw_char8>		DataCacheQueue;

NetMessageQueue			__g_msg_queue;
DataCacheQueue			__g_data_queue;
std::mutex				__g_data_mutex;

void on_read(const lw_char8 * buffer, lw_int32 recvSize)
{
	if (recvSize <= 0) return;
	if (NULL == buffer) return;

	const lw_uint32 NeMsgHeadSize = sizeof(NetHead);

	__g_data_queue.push(const_cast<char*>(buffer), recvSize);

	NetHead* pHead = nullptr;

	lw_uint32 msgSize = (lw_uint32)__g_data_queue.size();
	if (msgSize >= NeMsgHeadSize)
	{
		do
		{
			pHead = (NetHead*)__g_data_queue.front();
			if (nullptr != pHead && msgSize >= pHead->size)
			{
				pHead->debug();
				lw_char8* pData = (__g_data_queue.front() + NeMsgHeadSize);

				NetMessage* smsg = NetMessage::createMessage();
				if (smsg)
				{
					smsg->setContent(pHead, pData, pHead->size - NeMsgHeadSize);
					smsg->Status = SocketStatus_RECV;
					__g_data_queue.pop(pHead->size);
					
					//分发数据
					__g_msg_queue.push_back(smsg);

					{
						lw_int32 queueSize = 0;
						do
						{
							NetMessage* smsg = nullptr;
							{
								std::lock_guard < std::mutex > lock(__g_data_mutex);
								queueSize = __g_msg_queue.size();
								if (queueSize > 0)
								{
									smsg = __g_msg_queue.front();
									__g_msg_queue.pop_front();
								}
							}
							if (nullptr != smsg)
							{
								// 处理数据
// 								{
// 									UserInfo * pl = (UserInfo*)smsg->object;
// 									pl->debug();
// 								}

								{
									lm::UserInfo_msg msg1;
									msg1.ParseFromArray(smsg->object, smsg->objectSize);

									std::cout << msg1.u_age() << std::endl;
									std::cout << msg1.u_sex() << std::endl;
									std::cout << msg1.u_name() << std::endl;
									std::cout << msg1.u_address() << std::endl;
								}
								NetMessage::releaseMessage(smsg);
							}
						} while (queueSize > 0);
					}
					
				}
				else
				{
					printf("%s >> not a complete data packet [messageSize = %lu, pMessageHead->uMessageSize = %lu]",
						"1111111", msgSize, pHead->size);
				}
				msgSize = (lw_uint32)__g_data_queue.size();
				//HNLOG_INFO("%s >> messageSize ---> end = [%lu]", _tag.c_str(), messageSize);
			}
		} while (msgSize >= pHead->size);
	}
}

void transform(std::function<void(NetMessage* socketMessage)> func)
{
	int queueSize = 0;
	do
	{
		NetMessage* socketMessage = nullptr;
		{
			std::lock_guard < std::mutex > autoLock(__g_data_mutex);
			queueSize = __g_msg_queue.size();
			if (queueSize > 0)
			{
				socketMessage = __g_msg_queue.front();
				__g_msg_queue.pop_front();
			}
		}
		if (nullptr != socketMessage)
		{
			func(socketMessage);
			NetMessage::releaseMessage(socketMessage);
		}
	} while (queueSize > 0);
	//Director::getInstance()->getScheduler()->pauseTarget(this);
}

lw_int32 send_data(lw_int32 sock, const lw_char8 *name, lw_uint32 u_cmd, lw_uint32 CheckCode, void* object, lw_int32 objectSize)
{
	ProtocolData protocolData;
	protocolData.createPackage(u_cmd, CheckCode, object, objectSize);
	return nn_send(sock, protocolData.getContent(), protocolData.getContentSize(), 0);
}

lw_int32 recv_data(lw_int32 sock, const lw_char8 *name)
{
	char *buf = NULL;
	lw_int32 result = nn_recv(sock, &buf, NN_MSG, 0);
	if (result > 0)
	{
		on_read(buf, result);
		nn_freemsg(buf);
	}
	else
	{

	}

	return result;
}

int pair_on_data(int sock, const char *name)
{
	int to = 100;
	assert(nn_setsockopt(sock, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to)) >= 0);
	while (1)
	{
		{
			recv_data(sock, name);
		}
		
/*		SLEEP(1);*/

// 		{
// 			UserInfo pl;
// 			pl.age = 30;
// 			pl.sex = 1;
// 			strcpy(pl.name, name);
// 			strcpy(pl.address, "大冲国际中心·5#1202");
// 			send_data(sock, name, 1, 10000, &pl, sizeof(pl));
// 		}

		{
			lm::UserInfo_msg msg;
			msg.set_u_age(30);
			msg.set_u_sex(1);
			msg.set_u_name("liwei");
			msg.set_u_address("guangdong shenzhen nanshan guangdong shenzhen nanshan guangdong shenzhen nanshan");

			char s[256] = { 0 };
			bool ret = msg.SerializePartialToArray(s, 256);

			send_data(sock, name, 1, 10000, s, strlen(s));
		}
	}
}

int server_node(const char *url)
{
	int sock = nn_socket(AF_SP, NN_PAIR);
	assert(sock >= 0);
	assert(nn_bind(sock, url) >= 0);

	pair_on_data(sock, NODE0);

	return nn_shutdown(sock, 0);
}

int client_node(const char *url)
{
	int sock = nn_socket(AF_SP, NN_PAIR);
	assert(sock >= 0);
	assert(nn_connect(sock, url) >= 0);

	pair_on_data(sock, NODE1);

	return nn_shutdown(sock, 0);
}

int main(int argc, char **argv)
{
	{
		lm::UserInfo_msg msg;
		msg.set_u_age(30);
		msg.set_u_sex(1);
		msg.set_u_name("liwei");
		msg.set_u_address("guangdong shenzhen nanshan");

		char s[256] = { 0 };
		bool ret = msg.SerializePartialToArray(s, 256);

		if (ret)
		{
			lm::UserInfo_msg msg1;
			msg1.ParseFromArray(s, 256);

			std::cout << msg1.u_age() << std::endl;
			std::cout << msg1.u_sex() << std::endl;
			std::cout << msg1.u_name() << std::endl;
			std::cout << msg1.u_address() << std::endl;
		}
	}

	if (argc == 2 && strncmp(NODE0, argv[1], strlen(NODE0)) == 0) {
		return server_node(SOCKET_ADDR);
	}
	else if (argc == 2 && strncmp(NODE1, argv[1], strlen(NODE1)) == 0) {
		return client_node(SOCKET_ADDR);
	}
	else {
		fprintf(stderr, "Usage: pair %s|%s <ARG> ...\n", NODE0, NODE1);
		return 1;
	}
}