#include "lw_xxtea.h"

#include "FileUtils.h"
#include "Data.h"

extern "C"
{
	#include "./ext/xxtea.h"
}

XXTea::XXTea(const char* k)
{
	this->k = k;
}

XXTea::~XXTea()
{
}

void XXTea::encrypt(const char* filename, const char* outfilename)
{
	Data d = FileUtils::getInstance()->getDataFromFile(filename);
	this->encrypt(d.getBytes(), d.getSize(), [outfilename](unsigned char* out, unsigned int len)
	{
		FileUtils::getInstance()->setDataToFile(outfilename, out, len);
	});
}

void XXTea::encrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func)
{
	unsigned int len;
	unsigned char* out;
	out = xxtea_encrypt(data, data_len, (unsigned char*)k.c_str(), k.size(), &len);
	func(out, len);
	free(out);
}

void XXTea::decrypt(const char* filename, const char* outfilename)
{
	Data d = FileUtils::getInstance()->getDataFromFile(filename);
	this->decrypt(d.getBytes(), d.getSize(), [outfilename](unsigned char* out, unsigned int len)
	{
		FileUtils::getInstance()->setDataToFile(outfilename, out, len);
	});
}

void XXTea::decrypt(unsigned char *data, unsigned int data_len, std::function<void(unsigned char * out, unsigned int len)> func)
{
	unsigned int len;
	unsigned char* out;
	out = xxtea_decrypt(data, data_len, (unsigned char*)k.c_str(), k.size(), &len);
	func(out, len);
	free(out);
}


