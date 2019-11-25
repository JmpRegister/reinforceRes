//
// Created by thr0wCat on 2019-11-22.
//

#ifndef REINFORCERES_JNITOOLS_H
#define REINFORCERES_JNITOOLS_H
#include <jni.h>


jstring getApkPath(JNIEnv* env, jobject application);
char* jstringToChar(JNIEnv* env, jstring jstr);
jobject getApplication(JNIEnv *env);

#endif //REINFORCERES_JNITOOLS_H
