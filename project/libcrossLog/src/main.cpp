#include <stdio.h>
#include "..\FastLog.h"

void test0()
{
	HNFastLogFile log(__FILE__, __FUNCTION__, __LINE__);

	log.logD(__LINE__, "%s", "000000000000000");
}

void test1()
{
	HNFastLogFile log(__FILE__, __FUNCTION__, __LINE__);

	log.logW(__LINE__, "%s", "111111111111111");
}

void test2()
{
	HNFastLogFile log(__FILE__, __FUNCTION__, __LINE__);

	log.logE(__LINE__, "%s", "22222222222222");
}

int main(int argc, char** argv)
{
	hn_start_fastlog();

	FASTLOG_DEBUG(__FILE__, __LINE__, "士大夫大师傅十分");
	FASTLOG_DEBUG(__FILE__, __LINE__, 100000);
	FASTLOG_DEBUG(__FILE__, __LINE__, 12313.123);

	{
		test0();
		test1();
		test2();
	}

	getchar();

	return 0;
}
