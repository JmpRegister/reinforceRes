//
// Created by thr0wCat on 2019-11-22.
//

#ifndef REINFORCERES_MYLOG_H
#define REINFORCERES_MYLOG_H

#include<android/log.h>

#define Tag "reinforceRes"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, Tag, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, Tag, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, Tag, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, Tag, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, Tag, __VA_ARGS__)

#endif //REINFORCERES_MYLOG_H
