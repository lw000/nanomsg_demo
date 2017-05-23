#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <event.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <err.h>
#include <unistd.h>


//客户端发送的内容的长度
#define BUFLEN 1024

//全局变量
struct event ev_accept;
int socket_fd;

//内部函数，只能被本文件中的函数调用
static short ListenPort = 8080;
static long ListenAddr = INADDR_ANY;//任意地址，值就是0
static int   MaxConnections = 2;//连接请求队列的最大长度

struct bufferq {
	//buffer
	u_char *buf;

	//buf的长度
	int len;

	//写操作的偏移，用于buf的重试输出
	int offset;

	/*指向队列的前一个和后一个元素*/
	TAILQ_ENTRY(bufferq) entries;
};

//客户数据结构
struct client {
	//两个事件对象
	struct event ev_read;
	struct event ev_write;

	//存储的数据队列
	TAILQ_HEAD(, bufferq) writeq;
};

//将一个socket设置成非阻塞模式
//不论什么平台编写网络程序，都应该使用NONBLOCK socket的方式。这样可以保证你的程序至少不会在recv/send/accept/connect这些操作上发生block从而将整个网络服务都停下来
int setnonblock(int fd)
{
	int flags;
	//fcntl()用来操作文件描述符的一些特性
	if ((flags = fcntl(fd, F_GETFL)) == -1) {
		return -1;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

//当客户端socket准备好写入时，libevent调用这个函数
void on_write(int fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	struct bufferq *bufferq;
	int len, wlen;

	// 将第一个元素移出写队列
	bufferq = TAILQ_FIRST(&client->writeq);
	if (bufferq == NULL) {
		return;
	}

	//写buffer里的数据，如果有一部分已经输出过了，只输出剩余的部分
	len = bufferq->len - bufferq->offset;
	wlen = write(fd, bufferq->buf + bufferq->offset, len);
	if (wlen == -1) {
		//写操作被打断或不能写入数据
		if (errno == EINTR || errno == EAGAIN) {
			event_add(&client->ev_write, NULL);
			return;
		} else {
			//其它的sock错误
			//close(fd);
			//free(client);
			err(1, "write");
			return;
		}
	} else if (wlen < len) {
		//只输出了一部分数据，更新偏移量，调整写入事件
		bufferq->offset += wlen;
		event_add(&client->ev_write, NULL);
		return;
	}

	//队列中移除
	TAILQ_REMOVE(&client->writeq, bufferq, entries);
	//回收calloc申请的内存
	//free(bufferq->buf);//不是calloc申请的内存不能free
	free(bufferq);
}
//这个函数当客户端的socket可读时由libevent调用
void on_read(int fd, short ev, void *arg)
{
	struct client *client = (struct client *)arg;
	u_char buf[BUFLEN];
	int len, wlen;
	struct bufferq *bufferq;

	memset(&buf, 0, sizeof(buf));
	//会把参数fd 所指的文件传送count个字节到buf指针所指的内存中
	len = read(fd, buf, sizeof(buf));
	if (len == 0) {
		/* 客户端断开连接，在这里移除读事件并且释放客户数据结构 */
		printf("disconnected\n");
		close(fd);
		event_del(&client->ev_read);
		free(client);
		return;
	} else if (len < 0) {
		/* 出现了其它的错误，在这里关闭socket，移除事件并且释放客户数据结构 */
		printf("socket fail %s\n", strerror(errno));
		close(fd);
		event_del(&client->ev_read);
		free(client);
		return;
	}
	//如果client发送来bye，则断开连接
	if (!strcmp(buf, "bye\r\n")) {
		close(fd);
		event_del(&client->ev_read);
		free(client);
		return;
	}

	bufferq = calloc(1, sizeof(*bufferq));
	if (bufferq == NULL) {
		close(fd);
		event_del(&client->ev_read);
		free(client);
		printf("malloc fail");
		return;
	}
	bufferq->buf = buf;
	bufferq->len = len;
	bufferq->offset = 0;
	//插入队列
	TAILQ_INSERT_TAIL(&client->writeq, bufferq, entries);

	//写事件
	event_add(&client->ev_write, NULL);
}

/*
   当有一个连接请求准备被接受时，这个函数将被libevent调用并传递给三个变量: 
   int fd:触发事件的文件描述符. 
   short event:触发事件的类型EV_TIMEOUT,EV_SIGNAL, EV_READ, or EV_WRITE. 
   void* :由arg参数指定的变量. 
*/
void on_accept(int fd, short ev, void *arg)
{
	int cfd;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int yes = 1;
	int retval;
	//为每个客户端建一个client而不是写成全局变量,从而可以接收多个请求
	struct client *client;

	//将从连接请求队列中获得连接信息，创建新的套接字，并返回该套接字的文件描述符。
	//新创建的套接字用于服务器与客户机的通信，而原来的套接字仍然处于监听状态。
	//该函数的第一个参数指定处于监听状态的流套接字
	cfd = accept(fd, (struct sockaddr *)&addr, &addrlen);
	if (cfd == -1) {
		printf("accept(): can not accept client connection");
		return;
	}
	if (setnonblock(cfd) == -1) {
		close(cfd);
		return;
	}

	//为新客户分配一个客户数据结构来保存这个客户状态
	client = calloc(1, sizeof(*client));
	if (client == NULL) {
		printf("calloc faild");
		close(cfd);
		return;
	}

	//设置与某个套接字关联的选项
	//参数二 IPPROTO_TCP:TCP选项
	//参数三 TCP_NODELAY 不使用Nagle算法 选择立即发送数据而不是等待产生更多的数据然后再一次发送
	//       更多参数TCP_NODELAY 和 TCP_CORK
	//参数四 新选项TCP_NODELAY的值
	if (setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
		printf("setsockopt(): TCP_NODELAY %s\n", strerror(errno));
		close(cfd);
		return;
	}

	event_set(&client->ev_read, cfd, EV_READ | EV_PERSIST, on_read, client);
	event_add(&client->ev_read, NULL);
	
	event_set(&client->ev_write, cfd, EV_WRITE, on_write, client);
	//初始化客户端队列
	TAILQ_INIT(&client->writeq);
	printf("Accepted connection from %s\n", inet_ntoa(addr.sin_addr));
}
int create_socket(void)
{
	struct sockaddr_in sa;

	//socket函数来创建一个能够进行网络通信的套接字。
	//第一个参数指定应用程序使用的通信协议的协议族，对于TCP/IP协议族，该参数置AF_INET;
	//第二个参数指定要创建的套接字类型
	//流套接字类型为SOCK_STREAM、数据报套接字类型为SOCK_DGRAM、原始套接字SOCK_RAW
	//第三个参数指定应用程序所使用的通信协议。
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		printf("socket(): can not create server socket\n");
		return -1;
	}
	if (setnonblock(socket_fd) == -1) {
		return -1;
	}

	//如何避免等待60秒之后才能重启服务
	//in case of 'address already in use' error message
	/*这个套接字选项通知内核，如果端口忙，但TCP状态位于 TIME_WAIT ，可以重用端口。
	  如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信息，指明"地址已经使用中"。
	  如果你的服务程序停止后想立即重启，而新套接字依旧使用同一端口，此时 SO_REUSEADDR 选项非常有用。
	  必须意识到，此时任何非期望数据到达，都可能导致服务程序反应混乱，不过这只是一种可能，事实上很不可能。
	*/
	int yes = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
		//perror()用来将上一个函数发生错误的原因输出到标准设备(stderr) 
		perror("setsockopt failed");
		return -1;
	}


	//清空内存数据
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	//htons将一个无符号短整型数值转换为网络字节序
	sa.sin_port = htons(ListenPort);
	//htonl将主机的无符号长整形数转换成网络字节顺序
	sa.sin_addr.s_addr = htonl(ListenAddr);

	//(struct sockaddr*)&sa将sa强制转换为sockaddr类型的指针
	/*struct sockaddr 
		数据结构用做bind、connect、recvfrom、sendto等函数的参数，指明地址信息。
		但一般编程中并不直接针对此数据结构操作，而是使用另一个与sockaddr等价的数据结构 struct sockaddr_in
		sockaddr_in和sockaddr是并列的结构，指向sockaddr_in的结构体的指针也可以指向
		sockadd的结构体，并代替它。也就是说，你可以使用sockaddr_in建立你所需要的信息,
		在最后用进行类型转换就可以了
	*/
	//bind函数用于将套接字绑定到一个已知的地址上
	if (bind(socket_fd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		close(socket_fd);
		printf("bind(): can not bind server socket");
		return -1;
	}
	
	//执行listen 之后套接字进入被动模式
	//MaxConnections 连接请求队列的最大长度,队列满了以后，将拒绝新的连接请求
	if (listen(socket_fd, MaxConnections) == -1) {
		printf("listen(): can not listen server socket");
		close(socket_fd);
		return -1;
	}

	/*
	   event_set的参数：
	   + 参数1:  为要创建的event
	   + 参数2:  file descriptor，创建纯计时器可以设置其为-1，即宏evtimer_set定义的那样
	   + 参数3:  设置event种类，常用的EV_READ, EV_WRITE, EV_PERSIST, EV_SIGNAL, EV_TIMEOUT，纯计时器设置该参数为0
	   + 参数4:  event被激活之后触发的callback函数
	   + 参数5:  传递给callback函数的参数
	   备注：
			如果初始化event的时候设置其为persistent的(设置了EV_PERSIST)，
			则使用event_add将其添加到侦听事件集合后(pending状态)，
			该event会持续保持pending状态，即该event可以无限次参加libevent的事件侦听。
			每当其被激活触发callback函数执行之后，该event自动从active转回为pending状态，
			继续参加libevent的侦听(当激活条件满足，又可以继续执行其callback)。
			除非在代码中使用event_del()函数将该event从libevent的侦听事件集合中删除。
			如果不通过设置EV_PERSIST使得event是persistent的，需要在event的callback中再次调用event_add
			(即在每次pending变为active之后，在callback中再将其设置为pending)
	 */
	event_set(&ev_accept, socket_fd, EV_READ | EV_PERSIST, on_accept, NULL);
	//将event添加到libevent侦听的事件集中
	if (event_add(&ev_accept, NULL) == -1) {
		printf("event_add(): can not add accept event into libevent");
		close(socket_fd);
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int retval;
	
	//初始化event base 使用默认的全局current_base
	event_init();
	
	retval = create_socket();
	if (retval == -1) {
		exit(-1);
	}
	//event_dispatch() 启动事件队列系统，开始监听（并接受）请求
	event_dispatch();
	
	return 0;
}
