#include "lw_base64.h"

extern "C"
{
#include "./ext/base64.h"
}

Base64::Base64()
{

}

Base64::~Base64()
{
}

void Base64::encrypt(unsigned char *data, unsigned int data_len, std::function<void(char * out, unsigned int len)> func)
{
	unsigned int len;
	char* out;
	len = base64Encode(data, data_len, &out);
	func(out, len);
	free(out);
}

void Base64::decrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func)
{
	unsigned int len;
	unsigned char* out;
	len = base64Decode(data, data_len, &out);
	out[len] = 0;
	func(out, len);
	free(out);
}


