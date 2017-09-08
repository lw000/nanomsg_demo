#include "NetHead.h"

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#include <iosfwd>

namespace lwstar
{
	static unsigned short __make_package_version(unsigned char major, unsigned char minor) {
		unsigned int version = major;
		version = (version << 8);
		version = (version) | minor;
		return version;
	}

	static void __package_version(unsigned short version, unsigned char &major, unsigned char &minor) {
		minor = (version & 0x00FF);

		version = (version >> 8);
		major = (version & 0x00FF);
	}

	std::ostream& operator<<(std::ostream & os, tagNetHead & o)
	{
		os << o.debug();
		return os;
	}

	tagNetHead::tagNetHead()
	{
		this->size = 0;			// 数据包大小
		this->cmd = 0;			// 指令
		this->ctime = 0;		// 发送时间
		this->v = __make_package_version(1, 1);
	}

	std::string tagNetHead::debug()
	{
		char buf[512];
		unsigned char v1;
		unsigned char v2;
		__package_version(this->v, v1, v2);
		sprintf(buf, "NetHead [(size: %d), (cmd: %d), (time: %u), (v: %d.%d)]", this->size, this->cmd, this->ctime, v1, v2);
		return std::string(buf);
	}
}
