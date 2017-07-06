#include "FastLog.h"

#if defined(ANDROID)
#include <jni.h>
#include <android/log.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////
#define LOG_BUF_SIZE 8*1024

////////////////////////////////////////////////////////////////////////////////////////////////
enum HNLOG_LEVEL
{
	LOG_DEBUG = 0,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR
};

////////////////////////////////////////////////////////////////////////////////////////////////////
void hn_config_fastlog(const char* logpath);
void hn_start_fastlog();
void hn_stop_fastlog();

//////////////////////////////////////////////////////////////////////////////////////////
#if defined(ANDROID)
extern "C" {
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnConfigFastLog(JNIEnv *env, jobject obj, jstring jpath);
	
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnStartFastLog(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnStopFastLog(JNIEnv *env, jobject obj);

	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogD(JNIEnv *env, jobject obj, jstring log);
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogI(JNIEnv *env, jobject obj, jstring log);
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogW(JNIEnv *env, jobject obj, jstring log);
	JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogE(JNIEnv *env, jobject obj, jstring log);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnConfigFastLog(JNIEnv *env, jobject obj, jstring jpath)
{
	const char* cpath = env->GetStringUTFChars(jpath, 0);
	hn_config_fastlog(cpath);
	env->ReleaseStringUTFChars(jpath, cpath);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnStartFastLog(JNIEnv *env, jobject obj)
{
	hn_start_fastlog();
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogD(JNIEnv *env, jobject obj, jstring jlog)
{
	const char* clog = env->GetStringUTFChars(jlog, 0);
	LOGD(clog);
	env->ReleaseStringUTFChars(jlog, clog);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogI(JNIEnv *env, jobject obj, jstring jlog)
{
	const char* clog = env->GetStringUTFChars(jlog, 0);
	LOGI(clog);
	env->ReleaseStringUTFChars(jlog, clog);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogW(JNIEnv *env, jobject obj, jstring jlog)
{
	const char* clog = env->GetStringUTFChars(jlog, 0);
	LOGW(clog);
	env->ReleaseStringUTFChars(jlog, clog);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnPrintLogE(JNIEnv *env, jobject obj, jstring jlog)
{
	const char* clog = env->GetStringUTFChars(jlog, 0);
	LOGE(clog);
	env->ReleaseStringUTFChars(jlog, clog);
}

JNIEXPORT void JNICALL Java_com_cross_log_CrossLog_hnStopFastLog(JNIEnv *env, jobject obj)
{
	hn_start_fastlog();
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
void hn_config_fastlog(const char* logpath)
{
	ILog4zManager::getInstance()->setLoggerPath(LOG4Z_MAIN_LOGGER_ID, logpath);
}

void hn_start_fastlog()
{
	ILog4zManager::getInstance()->start();
}

void hn_stop_fastlog()
{
	ILog4zManager::getInstance()->stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int __hn_log_print(int level, const char* file, int line, const char* format, va_list args)
{
	char buf[LOG_BUF_SIZE+2] = { 0 };
	int c = vsnprintf(buf, LOG_BUF_SIZE - 2, format, args);
	switch (level)
	{
	case LOG_DEBUG: FASTLOG_DEBUG(file, line, buf); break;
	case LOG_INFO: 	FASTLOG_INFO(file, line, buf); break;
	case LOG_WARN: 	FASTLOG_WARN(file, line, buf); break;
	case LOG_ERROR: FASTLOG_ERROR(file, line, buf); break;
	default:
		break;
	}
	return c;
}

HNFastLogFile::HNFastLogFile(const char* file, const char* func, int line) : _file(file), _func(func), _line(line)
{
	logD(_line, "---------------------< enter >---------------------");
}

HNFastLogFile::~HNFastLogFile()
{
	logD(_line, "---------------------< exit >---------------------");
}

void HNFastLogFile::logD(int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	__hn_log_print(LOG_DEBUG, std::string(_file + ":" + _func).c_str(), line, format, args);
	va_end(args);
}

void HNFastLogFile::logI(int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	__hn_log_print(LOG_INFO, std::string(_file + ":" + _func).c_str(), line, format, args);
	va_end(args);
}

void HNFastLogFile::logW(int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	__hn_log_print(LOG_WARN, std::string(_file + ":" + _func).c_str(), line, format, args);
	va_end(args);
}

void HNFastLogFile::logE(int line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	__hn_log_print(LOG_ERROR, std::string(_file + ":" + _func).c_str(), line, format, args);
	va_end(args);
}