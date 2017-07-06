#include "log4z_android_ios_ext.h"

#ifdef __APPLE__

#endif

#if defined(ANDROID)
#include <jni.h>
#include <android/log.h>

#define  HNLOG_TAG    "hnlog"
#define  HNLOGD_PRINT(f, ...)  __android_log_print(ANDROID_LOG_DEBUG,HNLOG_TAG,f, ##__VA_ARGS__)
#define  HNLOGI_PRINT(f, ...)  __android_log_print(ANDROID_LOG_INFO,HNLOG_TAG,f, ##__VA_ARGS__)
#define  HNLOGW_PRINT(f, ...)  __android_log_print(ANDROID_LOG_WARN,HNLOG_TAG,f, ##__VA_ARGS__)
#define  HNLOGE_PRINT(f, ...)  __android_log_print(ANDROID_LOG_ERROR,HNLOG_TAG,f, ##__VA_ARGS__)
#define  HNLOGF_PRINT(f, ...)  __android_log_print(ANDROID_LOG_FATAL,HNLOG_TAG,f, ##__VA_ARGS__)

#endif

void hook_log_func(const char* log)
{
#if defined(ANDROID)
	HNLOGD_PRINT("%s", log);
#endif

#if defined(__APPLE__)
	NSString* log = [NSString stringWithFormat : @"%s", log];
	NSLog(@"%@", log);
#endif
}