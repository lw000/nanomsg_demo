#ifndef __AUTO_LOG_H__
#define __AUTO_LOG_H__

#include <string>

#include "log4z.h"
using namespace zsummer::log4z;

void hn_config_fastlog(const char* path);
void hn_start_fastlog();
void hn_stop_fastlog();

/////////////////////////////////////////////////////////////////////////////////////////
#define FASTLOG_TRACE(f, n, text) LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_TRACE, f, n, text)
#define FASTLOG_DEBUG(f, n, text) LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_DEBUG, f, n, text)
#define FASTLOG_INFO(f, n, text)  LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_INFO, f, n, text)
#define FASTLOG_WARN(f, n, text)  LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_WARN, f, n, text)
#define FASTLOG_ERROR(f, n, text) LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_ERROR, f, n, text)
#define FASTLOG_ALARM(f, n, text) LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_ALARM, f, n, text)
#define FASTLOG_FATAL(f, n, text) LOG_STREAM(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_FATAL, f, n, text)

class HNFastLogFile
{
public:
	HNFastLogFile(const char* file, const char* func, int line);
	~HNFastLogFile();

public:
	void logD(int line, const char* format, ...);
	void logI(int line, const char* format, ...);
	void logW(int line, const char* format, ...);
	void logE(int line, const char* format, ...);

private:
	std::string _file;
	std::string _func;
	int _line;
};

#endif	//__AUTO_LOG_H__
