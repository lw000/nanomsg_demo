#include "lwutil.h"

#include <string.h>

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