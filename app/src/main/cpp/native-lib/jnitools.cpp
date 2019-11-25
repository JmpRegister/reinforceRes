//
// Created by thr0wCat on 2019-11-22.
//

#include <cstring>
#include <cstdlib>
#include "jnitools.h"
#include "mylog.h"



jobject getApplication(JNIEnv *env) {
    jobject application = NULL;
    jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
    if (activity_thread_clz != NULL) {
        jmethodID currentApplication = env->GetStaticMethodID(
                activity_thread_clz, "currentApplication", "()Landroid/app/Application;");
        if (currentApplication != NULL) {
            application = env->CallStaticObjectMethod(activity_thread_clz, currentApplication);
        } else {
            LOGD("Cannot find method: currentApplication() in ActivityThread.");
        }
        env->DeleteLocalRef(activity_thread_clz);
    } else {
        LOGD("Cannot find class: android.app.ActivityThread");
    }

    return application;
}

jstring getApkPath(JNIEnv* env, jobject application){

    LOGD("invoke native getApkPath");

    jclass cls = env->GetObjectClass(application);

    jmethodID mid = env->GetMethodID(cls, "getPackageResourcePath","()Ljava/lang/String;");

    jstring path = static_cast<jstring>(env->CallObjectMethod(application, mid)); //很神奇，在原生9 10模拟器上as调试时调用该处会art错误，但是通过log直接运行没问题

    if(path == NULL){
        LOGD("getApkPath cannot find base.apk path!");
    }
    LOGD("base.apk path is %s", jstringToChar(env, path));
    return path;
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}


