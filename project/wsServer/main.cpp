#include <event2\util.h>
#include "net.h"
#include "example.h"

int main(int argc, char **argv)
{
	if (argc < 2) return 0;
	
	SocketInit s;

	if (strcmp(argv[1], "s") == 0)
	{
		test_ws_server(argc, argv);
	}
	else if(strcmp(argv[1], "c") == 0)
	{
		test_ws_client(argc, argv);
	}
	else {

	}

	return 0;
}
