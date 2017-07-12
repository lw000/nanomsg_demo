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

#include "business.h"

#include "Message.h"
#include "platform.pb.h"
#include "..\libutils\lwutil.h"

using namespace LW;

static lw_int32 send_socket_data(lw_int32 sock, lw_int32 cmd, void* object, lw_int32 objectSize);
static lw_int32 recv_socket_data(lw_int32 sock);
static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata);

static void on_socket_recv(lw_int32 cmd, char* buf, lw_int32 bufsize, void* userdata)
{
	switch (cmd)
    {
        case 10000:
        {
            sc_userinfo *user = (sc_userinfo*)(buf);
            printf(" age: %d\n sex: %d\n name: %s\n address: %s\n",
                   user->age, user->sex, user->name, user->address);
        } break;
        case 10001:
        {
            platform::sc_msg_userinfo msg;
            msg.ParseFromArray(buf, bufsize);
            printf(" age: %d\n sex: %d\n name: %s\n address: %s\n",
                   msg.age(), msg.sex(), msg.name().c_str(), msg.address().c_str());
        }break;
        case 10002:
        {
            
        }break;
        default:
            break;
    }
}

static lw_int32 send_socket_data(lw_int32 sock, lw_int32 cmd, void* object, lw_int32 objectSize)
{
	lw_int32 c = 0;
	{
		LW_NET_MESSAGE* p = lw_create_net_message(cmd, object, objectSize);
		c = nn_send(sock, p->buf, p->size, 0);
		lw_free_net_message(p);
	}
	return c;
}

static lw_int32 recv_socket_data(lw_int32 sock)
{
    char *buf = NULL;
    lw_int32 c = nn_recv(sock, &buf, NN_MSG, 0);
    if (c > 0)
    {
		lw_parse_socket_data(buf, c, on_socket_recv, NULL);
        
        nn_freemsg(buf);
    }
    else
    {
        
    }
    
    return c;
}


/*  The server runs forever. */

int pub_server(const char *url)
{
	int fd;
	fd = nn_socket(AF_SP, NN_PUB);
	if (fd < 0) 
	{
		fprintf(stderr, "nn_socket: %s\n", nn_strerror(nn_errno()));
		return (-1);
	}

	if (nn_bind(fd, url) < 0)
	{
		fprintf(stderr, "nn_bind: %s\n", nn_strerror(nn_errno()));
		nn_close(fd);
		return (-1);
	}

	printf("pub server start ...\n");

	for (;;)
    {
		int i = rand() % 3;
		switch (i)
		{
		case 0:
		{
			sc_userinfo userinfo;
			userinfo.id = 4000001;
			userinfo.age = 30;
			userinfo.sex = 1;
			strcpy(userinfo.name, "liwei");
			strcpy(userinfo.address, "shanxi");
			send_socket_data(fd, 10000, &userinfo, sizeof(userinfo));
		} break;
		case 1:
		{
			platform::sc_msg_userinfo msg;
			msg.set_userid(4000001);
			msg.set_age(30);
			msg.set_sex(1);
			msg.set_name("heshanshan");
			msg.set_address("shenzhen");

			int len = (int)msg.ByteSizeLong();
			char s[256] = { 0 };
			bool ret = msg.SerializeToArray(s, len);
			//bool ret = msg.SerializePartialToArray(s, len);
			if (ret)
			{
				send_socket_data(fd, 10001, s, len);
			}
		} break;
		case 2:
		{
		} break;
		default:
			break;
		}
/*		lw_sleep(1);*/

	}
	/* NOTREACHED */

	nn_close(fd);

	return (-1);
}

int main(int argc, char **argv)
{
	if (argc < 3) return 0;

	int rc;
	if (strcmp(argv[2], "-s") == 0) {
		rc = pub_server(argv[1]);
	}
	else {
		fprintf(stderr, "Usage: %s <url> [-s]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	exit(rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
