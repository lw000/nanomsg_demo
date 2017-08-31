#include <event2\util.h>
#include "net.h"
#include "example.h"
#include "win32helpers\getopt.h"
#include <string>
#include "test.h"
#include "FileMerge.h"

static struct option options[] = {
	{ "type", required_argument, NULL, 't' },
	{ "port", required_argument, NULL, 'p' },
	{ NULL, 0, 0, 0 }
};

int main(int argc, char **argv)
{
	if (argc < 5) return 0;
	int n;
	int port;
	std::string t;
	n = 0;
	port = 5000;
	while (n >= 0) {
		n = getopt_long(argc, argv, "t:p:", options, NULL);
		if (n < 0)
			continue;
		switch (n) {
		case 't': {
			t.assign(optarg);
		} break;
		case 'p': {
			port = atoi(optarg);
		} break;
		default:
			break;
		}
	};
	
	test_main();

// 	FileMerge merge;
// 	merge.merge("E:/lw000/cpp/nanomsg_demo/project/wsServer/test.c");
// 	unsigned char* data = merge.getData("E:/lw000/cpp/nanomsg_demo/project/wsServer/test.c");
// 	merge.releaseData(data);

	SocketInit s;

	if (t.compare("s") == 0)
	{
		test_ws_server(port);
	}
	if (t.compare("c") == 0)
	{
		test_ws_client(port);
	}
	else {

	}

	return 0;
}
