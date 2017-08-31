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

#ifdef _WIN32
#include <winsock.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "socket_core.h"

#include "platform.pb.h"
#include "common_type.h"
#include "command.h"
#include "nanomsg_socket.h"

using namespace LW;

class Client : public NanomsgSocket
{
public:
	Client() {
	}

	virtual ~Client() {
	}

private:
	virtual void onRecv(lw_int32 cmd, char* buf, lw_int32 bufsize) override {
		switch (cmd)
		{
		case cmd_heart_beat: {
			platform::msg_heartbeat msg;
			msg.ParseFromArray(buf, bufsize);
			printf("heartBeat[%d]\n", msg.time());
		}break;
		default:
			break;
		}
	}
};

Client __g_client;

/*  The client runs in a loop, displaying the content. */

int client(const char *url)
{
	int fd;
	fd = __g_client.create(NN_SUB);

	if (fd < 0) {
		return (-1);
	}

	if (__g_client.connect(url) < 0) {
		return (-1);
	}

	/*  We want all messages, so just subscribe to the empty value. */

	if (__g_client.setsockopt(NN_SUB, NN_SUB_SUBSCRIBE, "", 0) < 0) {
		return (-1);
	}
	
	do
	{
		__g_client.recv();

	} while (1);

	__g_client.close();

	return (-1);
}

int main(int argc, char **argv)
{
	int rc;
	if (argc == 2) {
		rc = client(argv[1]);
	}
	else {
		fprintf(stderr, "Usage: %s <url> [-s]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
