#ifndef __lw_base64_h__
#define __lw_base64_h__

#include <string>
#include <functional>

class Base64
{
public:
	Base64();
	~Base64();

public:
	void encrypt(const char* filename, std::function<void(char * out, unsigned int len)> func);
	void encrypt(unsigned char *data, unsigned int data_len, std::function<void(char * out, unsigned int len)> func);
	
	void decrypt(const char* filename, std::function<void(unsigned char * out, unsigned int len)> func);
	void decrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func);
};


#endif	// __lw_base64_h__
