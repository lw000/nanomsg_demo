#ifndef __LW_UTIL_H__
#define __LW_UTIL_H__

#include <vector>
#include <string>
#include <unordered_map>
#include <list>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <functional>

#ifdef _WIN32
#define lw_sleep(seconds) SleepEx(seconds * 1000, 1);
#else
#define lw_sleep(seconds) sleep(seconds);
#endif

struct KV
{
	char *k;
	char *v;
};

class KVPragma
{
public:
	KVPragma();
	~KVPragma();

public:
	int parse_url(const char* data);
	char* find_value(const char* key);
	void printf(std::function<void(KV*)> func);

private:
	std::vector<KV*> _kv;
};

char * lw_strtok_r(char *s, const char *delim, char **state);

std::vector<std::string> split(const char* str, const char* pattern);

std::unordered_map<std::string, std::string> split_url_pragma_data(const char* str);

void trim(char* src, char* dest);
void trim_l(char* src, char* dest);
void trim_r(char* src, char* dest);

#if defined(_WIN32)

//UTF-8到GB2312转换
char* utf8_to_gbk(const char* utf8);
//GB2312到UTF-8转换
char* gbk_to_utf8(const char* gb2312);

std::wstring StringUtf8ToWideChar(const std::string& strUtf8);
std::string StringWideCharToUtf8(const std::wstring& strWideChar);
std::string UTF8StringToMultiByte(const std::string& strUtf8);

#endif

#if defined(_WIN32)
#define U2G(v)	utf8_to_gbk(v);
#define G2U(v)	gbk_to_utf8(v);
#else
#define U2G(v)
#define G2U(v)
#endif

#endif // !__LW_UTIL_H__
