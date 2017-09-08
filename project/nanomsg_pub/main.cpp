/*
Copyright 2016 Garrett D'Amore <garrett@damore.org>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

"nanomsg" is a trademark of Martin Sustrik
*/

/*  This program serves as an example for how to write a simple PUB SUB

service, The server is just a single threaded for loop which broadcasts

messages to clients, every so often.  The message is a binary format

message, containing two 32-bit unsigned integers.  The first is UNIX time,

and the second is the number of directly connected subscribers.



The clients stay connected and print a message with this information

along with their process ID to standard output.



To run this program, start the server as pubsub_demo <url> -s

Then connect to it with the client as pubsub_demo <url>

For example:



% ./pubsub_demo tcp://127.0.0.1:5555 -s &

% ./pubsub_demo tcp://127.0.0.1:5555 &

% ./pubsub_demo tcp://127.0.0.1:5555 &

11:23:54 <pid 1254> There are 2 clients connected.

11:24:04 <pid 1255> There are 2 clients connected.

..

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <nn.h>
#include <pubsub.h>
#include <transport.h>

#ifdef _WIN32
#include <winsock.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "socket_core.h"

#include <thread>

#include "common_type.h"
#include "nanomsg_socket.h"
#include "message_queue.h"
#include "lw_util.h"
#include "FastLog.h"

#include "command.h"
#include "game.pb.h"
#include "platform.pb.h"


using namespace LW; 

struct pthread_args {
	int destroy_flag;
	int connection_flag;
};

struct push_pthread_args {
	struct pthread_args *pargs;
	std::string url;
};

SimpleMessageQueue __g_msg_queue;

class PushServer : public NanomsgSocket
{
public:
	PushServer() {
	}

	virtual ~PushServer() {
	}

public:
	virtual void onRecv(lw_int32 cmd, char* buf, lw_int32 bufsize) override {

	}
};

/*  The server runs forever. */

void pub_thread_server(void* args)
{
	struct push_pthread_args *pargs = (struct push_pthread_args *)args;

	PushServer serv;
	int fd = serv.create(AF_SP, NN_PUB);
	if (fd < 0) 
	{
		return;
	}

	if (serv.bind(pargs->url.c_str()) < 0)
	{
		return;
	}

	printf("pub server start ...\n");
	
	pargs->pargs->connection_flag = 1;

	for (;;)
    {
		if (!__g_msg_queue.empty())
		{
			Msgdata msg = __g_msg_queue.pop();
			{
				char s[512];
				sprintf(s, "pop size [%lld]", __g_msg_queue.size());
				LOGD(s);
			}
			serv.send(msg.getmtype(), msg.getmtext(), msg.getmtextl());
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	serv.shutdown();

	return;
}

static void *pthread_push_msgdata(void *args)
{
	struct pthread_args *pargs = (struct pthread_args *)args;

	//* waiting for connection with server done.*/
	while (!pargs->connection_flag)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	while (!pargs->destroy_flag)
	{
		char* d = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		platform::msg_heartbeat msg;
		msg.set_time(time(NULL));
		msg.set_data(d);
		int c = msg.ByteSizeLong();
		std::unique_ptr<unsigned char[]> s(new unsigned char[c]());
		bool ret = msg.SerializeToArray(s.get(), c);
		if (ret)
		{
			Msgdata dmsg(cmd_heart_beat, s.get(), c);
			{
				__g_msg_queue.push(dmsg);
			}
		}

   		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return nullptr;
}

int main(int argc, char **argv)
{
	if (argc < 3) return 0;

	hn_start_fastlog();

	__g_msg_queue.createChannel();

	struct pthread_args* pargs = new struct pthread_args;
	pargs->connection_flag = 0;
	pargs->destroy_flag = 0;

	{
		std::thread a(pthread_push_msgdata, pargs);
		a.detach();
	}

	struct push_pthread_args* push_args = new struct push_pthread_args;
	push_args->pargs = pargs;
	push_args->url = argv[1];

	int rc;
	if (strcmp(argv[2], "-s") == 0) {
		std::thread a(pub_thread_server, push_args);
		a.detach();

		while (1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	else {
		fprintf(stderr, "Usage: %s <url> [-s]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
