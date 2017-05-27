#ifndef __LW_UTIL_H__
#define __LW_UTIL_H__

#include <vector>
#include <string>
#include <unordered_map>

char * lw_strtok_r(char *s, const char *delim, char **state);

std::vector<std::string> split(const char* str, const char* pattern);

std::unordered_map<std::string, std::string> split_url_pragma_data(const char* str);

#endif // !__LW_UTIL_H__
