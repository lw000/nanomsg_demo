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
	std::unordered_map<std::string, std::string> s;

	char *p = NULL;
	char *p1 = NULL;
	p = lw_strtok_r(const_cast<char*>(str), "&", &p1);
	while (p != NULL)
	{
		{
			char *q = NULL;
			char *q1 = NULL;
			std::string q2;
			std::string q3;

			q = lw_strtok_r(const_cast<char*>(p), "=", &q1);
			q2 = q;
			q = lw_strtok_r(NULL, "=", &q1);
			q3 = q;
			s[q2] = q3;
		}
		p = lw_strtok_r(NULL, "&", &p1);
	}
	return s;
}

