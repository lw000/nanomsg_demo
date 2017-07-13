#include "user_session.h"

UserSession::UserSession(TYPE c) : SocketSession(c)
{

}

UserSession::~UserSession()
{
}

void UserSession::setuid(unsigned int uid)
{
	this->uid = uid;
}

void UserSession::setExt(const std::string& ext)
{
	this->ext = ext;
}
