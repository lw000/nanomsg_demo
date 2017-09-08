#include "message_queue.h"

#include <string.h>

// from openssl copy

static unsigned long lw_hash_code(const char* c)
{
	unsigned long ret = 0;
	long n;
	unsigned long v;
	int r;

	if ((c == NULL) || (*c == '\0'))
		return(ret);

	/*
	unsigned char b[16];
	MD5(c,strlen(c),b);
	return(b[0]|(b[1]<<8)|(b[2]<<16)|(b[3]<<24));
	*/

	n = 0x100;
	while (*c)
	{
		v = n | (*c);
		n += 0x100;
		r = (int)((v >> 2) ^ v) & 0x0f;
		ret = (ret << r) | (ret >> (32 - r));
		ret &= 0xFFFFFFFFL;
		ret ^= v*v;
		c++;
	}
	return((ret >> 16) ^ ret);
}

Msgdata::Msgdata() {
	this->mtype = 0;
	this->mtext_l = 0;
	this->mtext = new unsigned char[1];
	this->mtext[0] = '\0';
}

Msgdata::Msgdata(int mtype, unsigned char* mtext, unsigned long mtext_l) {
	this->mtype = mtype;
	this->ctime = time(NULL);
	if (mtext == NULL) {
		this->mtext_l = 0;
		mtext = new unsigned char[1];
		mtext[0] = '\0';
	}
	else {
		this->mtext_l = mtext_l;
		this->mtext = new unsigned char[this->mtext_l];
		memcpy(this->mtext, mtext, this->mtext_l);
	}
}

Msgdata::~Msgdata() {
	if (this->mtext != NULL) { 
		delete[] this->mtext; this->mtext = NULL;
	}
}

Msgdata::Msgdata(const Msgdata& other) {
	this->mtype = other.mtype;
	this->mtext_l = other.mtext_l;
	this->mtext = new unsigned char[other.mtext_l];
	memcpy(this->mtext, other.mtext, this->mtext_l);
}

Msgdata::Msgdata(Msgdata&& other) {
	this->mtext_l = other.mtext_l;
	this->mtype = other.mtype;

	//���ܴ����ڴ�й¶
	//if (this->mtext != NULL) { delete[] this->mtext; this->mtext = NULL; }

	this->mtext = other.mtext;
	other.mtext = NULL;
}

Msgdata& Msgdata::operator=(const Msgdata& other) {
	if (this != &other) {
		this->mtext_l = other.mtext_l;
		this->mtype = other.mtype;

		if (mtext != NULL) { delete[] mtext; mtext = NULL; }

		this->mtext = new unsigned char[other.mtext_l];
		memcpy(this->mtext, other.mtext, this->mtext_l);
	}
	return *this;
}

Msgdata& Msgdata::operator=(Msgdata&& other) {
	if (this != &other) {
		this->mtext_l = other.mtext_l;
		this->mtype = other.mtype;

		if (mtext != NULL) { delete[] mtext; mtext = NULL; }

		this->mtext = other.mtext;
		other.mtext = NULL;
	}
	return *this;
}

long Msgdata::getmtype() const {
	return this->mtype;
}

unsigned char* Msgdata::getmtext() const {
	return this->mtext;
}

long Msgdata::getmtextl() const {
	return this->mtext_l;
}

static const Msgdata NULL_MSG;

//////////////////////////////////////////////////////////////////////////////////////////
SimpleMessageBroker::SimpleMessageBroker()
{
}

SimpleMessageBroker::~SimpleMessageBroker()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
SimpleMessageChannel::SimpleMessageChannel()
{
}

SimpleMessageChannel::~SimpleMessageChannel()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
SimpleMessageQueue::SimpleMessageQueue() {
	_msg_total = 0;
}

SimpleMessageQueue::~SimpleMessageQueue() {
}

int SimpleMessageQueue::createChannel(const char* channel) {
	if (channel != NULL) {
		unsigned long hashcode = lw_hash_code(channel);
		auto v = _msg_queue.find(hashcode);
		if (v != _msg_queue.end()) {
			_msg_queue[hashcode];
		}
	}
	else {
		_msg_queue[0];
	}

	return 0;
}

void SimpleMessageQueue::releaseChannel(const char* channel) {

}

bool SimpleMessageQueue::empty() {
	return (_msg_total == 0);
}

long long SimpleMessageQueue::size() {
	return _msg_total;
}

void SimpleMessageQueue::push(const Msgdata& msg, const char* channel) {
	{
		std::lock_guard<std::mutex> l(_m);
		if (channel == NULL) {
			_msg_queue.at(0).push(msg);
		}
		else {
			unsigned long hashcode = lw_hash_code(channel);
			_msg_queue.at(hashcode).push(msg);
		}
		_msg_total += 1;
	}
}

Msgdata SimpleMessageQueue::pop(const char* channel) {
	if (channel == NULL) {
		if (!_msg_queue.at(0).empty()) {
			_msg_total -= 1;
			{
				std::lock_guard<std::mutex> l(_m);
				Msgdata msg = _msg_queue.at(0).front();
				_msg_queue.at(0).pop();
				return msg;
			}
		}
	}
	else {
		unsigned long hashcode = lw_hash_code(channel);
		if (!_msg_queue.at(hashcode).empty()) {
			_msg_total -= 1;
			{
				std::lock_guard<std::mutex> l(_m);
				Msgdata msg = _msg_queue.at(hashcode).front();
				_msg_queue.at(hashcode).pop();
				return msg;
			}
		}
	}
	return NULL_MSG;
}
