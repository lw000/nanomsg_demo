#include "lwutil.h"
#include <string.h>

#if defined(_WIN32)

#include <windows.h>

unsigned long hash_code(const char* c)
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

std::wstring StringUtf8ToWideChar(const std::string& strUtf8)
{
	std::wstring ret;
	if (!strUtf8.empty())
	{
		int nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, nullptr, 0);
		if (nNum)
		{
			WCHAR* wideCharString = new WCHAR[nNum + 1];
			wideCharString[0] = 0;

			nNum = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wideCharString, nNum + 1);

			ret = wideCharString;
			delete[] wideCharString;
		}
		else
		{
			printf("Wrong convert to WideChar code:0x%x", GetLastError());
		}
	}
	return ret;
}

std::string StringWideCharToUtf8(const std::wstring& strWideChar)
{
	std::string ret;
	if (!strWideChar.empty())
	{
		int nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* utf8String = new char[nNum + 1];
			utf8String[0] = 0;

			nNum = WideCharToMultiByte(CP_UTF8, 0, strWideChar.c_str(), -1, utf8String, nNum + 1, nullptr, FALSE);

			ret = utf8String;
			delete[] utf8String;
		}
		else
		{
			printf("Wrong convert to Utf8 code:0x%x", GetLastError());
		}
	}

	return ret;
}

std::string UTF8StringToMultiByte(const std::string& strUtf8)
{
	std::string ret;
	if (!strUtf8.empty())
	{
		std::wstring strWideChar = StringUtf8ToWideChar(strUtf8);
		int nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, nullptr, 0, nullptr, FALSE);
		if (nNum)
		{
			char* ansiString = new char[nNum + 1];
			ansiString[0] = 0;

			nNum = WideCharToMultiByte(CP_ACP, 0, strWideChar.c_str(), -1, ansiString, nNum + 1, nullptr, FALSE);

			ret = ansiString;
			delete[] ansiString;
		}
		else
		{
			printf("Wrong convert to Ansi code:0x%x", GetLastError());
		}
	}

	return ret;
}

char* utf8_to_gbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

//GB2312µ½UTF-8µÄ×ª»»
char* gbk_to_utf8(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

static double getfreq(void)
{
	LARGE_INTEGER freq;
	if (!QueryPerformanceFrequency(&freq)) return 0.0;
	return (double)freq.QuadPart;
}

static double f = -1.0;

double gettime(void)
{
	LARGE_INTEGER t;
	if (f < 0.0) f = getfreq();
	if (f == 0.0) return (double)GetTickCount() / 1000.;
	else
	{
		QueryPerformanceCounter(&t);
		return (double)t.QuadPart / f;
	}
}

#else
#include <stdlib.h>
#include <sys/time.h>

double gettime(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL) < 0) return 0.0;
	else return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.);
}

#endif

char * lw_strtok_r(char *s, const char *delim, char **state) {
	char *cp, *start;
	start = cp = s ? s : *state;
	if (!cp)
		return NULL;
	while (*cp && !strchr(delim, *cp))
		++cp;
	if (!*cp) {
		if (cp == start)
			return NULL;
		*state = NULL;
		return start;
	}
	else {
		*cp++ = '\0';
		*state = cp;
		return start;
	}
}

KVQueryUrlValue::KVQueryUrlValue()
{
	_kv.clear();
}

KVQueryUrlValue::~KVQueryUrlValue()
{
	std::list<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter)
	{
		KV *_pkv = *iter;
		free(_pkv->k);
		free(_pkv->v);
		free(_pkv);
	}
}

int KVQueryUrlValue::parse(const char* data)
{
	_kv.clear();

	char *p = const_cast<char*>(data);
	char *p0 = NULL;
	char *p1 = NULL;
	p0 = lw_strtok_r(p, "&", &p1);
	if (p0 == NULL) return -1;
	while (p0 != NULL)
	{
		char *q = NULL;
		char *q1 = NULL;

		std::string k;
		std::string v;

		q = lw_strtok_r(const_cast<char*>(p0), "=", &q1);
		if (q == NULL) return -1;
		k = q;
		q = lw_strtok_r(NULL, "=", &q1);
		if (q == NULL) return -1;
		v = q;

		KV *_pkv = (KV*)malloc(sizeof(KV));
		_pkv->k = (char*)::malloc(k.size() + 1);
		_pkv->v = (char*)::malloc(v.size() + 1);
		strcpy(_pkv->k, k.c_str());
		strcpy(_pkv->v, v.c_str());

		_kv.push_back(_pkv);

		p0 = lw_strtok_r(NULL, "&", &p1);
	}
	return 0;
}

char* KVQueryUrlValue::find(const char* key)
{
	std::list<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter)
	{
		KV *_pkv = *iter;
		if (strcmp(_pkv->k, key) == 0)
		{
			return _pkv->v; break;
		}
	}
	return NULL;
}

void KVQueryUrlValue::each(std::function<void(KV*)> func)
{
	std::list<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); ++iter)
	{
		KV *_pkv = *iter;
		func(_pkv);
	}
}

std::vector<std::string> split(const char* str, const char* pattern)
{
	char *p = NULL;
	char *p1 = NULL;
	p = lw_strtok_r(const_cast<char*>(str), pattern, &p1);
	std::vector<std::string> s;
	while (p != NULL)
	{
		s.push_back(p);
		p = lw_strtok_r(NULL, pattern, &p1);
	}
	return s;
}

std::unordered_map<std::string, std::string> split_url_pragma_data(const char* str)
{
	char *p = const_cast<char*>(str);
	char *p0 = NULL;
	char *p1 = NULL;
	p0 = lw_strtok_r(p, "&", &p1);
	if (p0 == NULL)
	{
		return std::unordered_map<std::string, std::string>();
	}

	std::unordered_map<std::string, std::string> s;
	while (p0 != NULL)
	{
		{
			char *q = NULL;
			char *q1 = NULL;
			std::string k;
			std::string v;

			q = lw_strtok_r(const_cast<char*>(p0), "=", &q1);
			k = q;
			q = lw_strtok_r(NULL, "=", &q1);
			v = q;
			s[k] = v;
		}
		p0 = lw_strtok_r(NULL, "&", &p1);
	}
	return s;
}

void trim(char* src, char* dest)
{
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*ps == ' ') { ++ps;}

	while (*pe == ' ') { --pe;}

	char* p = dest;
	while (ps <= pe)
	{
		*p++ = *ps++;
	}

	*p = '\0';
}

void trim_l(char* src, char* dest)
{
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*ps == ' ') { ++ps; }

	char* p = dest;
	while (ps <= pe)
	{
		*p++ = *ps++;
	}

	*p = '\0';
}

void trim_r(char* src, char* dest)
{
	char* ps = src;
	char* pe = src + strlen(src) - 1;

	while (*pe == ' ') { --pe; }

	char* p = dest;
	while (ps <= pe)
	{
		*p++ = *ps++;
	}

	*p = '\0';
}