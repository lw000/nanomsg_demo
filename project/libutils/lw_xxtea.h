#ifndef __lw_xxtea_h__
#define __lw_xxtea_h__

#include <string>
#include <functional>

class XXTea final
{
public:
	XXTea(const char* k);
	~XXTea();

public:
	void encrypt(const char* filename, const char* outfilename);
	void encrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func);

	void decrypt(const char* filename, const char* outfilename);
	void decrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func);

private:
	std::string k;
};

#endif	// __lw_xxtea_h__
