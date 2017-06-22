#include "lwutil.h"

#include <string.h>

#ifdef __WIN32
#include <windows.h>
#endif // __WIN32

char* U2G(const char* utf8)
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
char* G2U(const char* gb2312)
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

KVPragma::KVPragma()
{
	_kv.clear();
}

KVPragma::~KVPragma()
{
	std::vector<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); iter++)
	{
		KV *_pkv = *iter;
		free(_pkv->k);
		free(_pkv->v);
		free(_pkv);
	}
}

int KVPragma::parse_url(const char* data)
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

char* KVPragma::find_value(const char* key)
{
	std::vector<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); iter++)
	{
		KV *_pkv = *iter;
		if (strcmp(_pkv->k, key) == 0)
		{
			return _pkv->v; break;
		}
	}
	return NULL;
}

void KVPragma::printf(std::function<void(KV*)> func)
{
	std::vector<KV*>::iterator iter = _kv.begin();
	for (; iter != _kv.end(); iter++)
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