#ifndef __user_session_H__
#define __user_session_H__

#include "base_type.h"

#include "socket_session.h"

class UserSession : public SocketSession
{
private:
	unsigned int uid;
	std::string ext;

public:
	UserSession(TYPE c);
	~UserSession();

public:
	void setuid(unsigned int uid);
	void setExt(const std::string& ext);
};


#endif // !__user_session_H__