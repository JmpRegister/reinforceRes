#include <jni.h>
#include <string>
#include "../FAInHook/FAInHook.h"
#include "native-lib.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#include "MyZipEntry.h"
#include "FileMap.h"
#include "mylog.h"
#include "androiddef.h"
#include "jnitools.h"


typedef FILE* (*FOPEN)(const char *path, const char *mode);
typedef int (*OPEN)(const char *path, int oflags,  mode_t mode);
typedef AAsset* (*AASSETMANAGER_OPEN)(AAssetManager* mgr, const char* filename, int mode);
typedef ssize_t (*READ)(int fd, void *buf, size_t count);
typedef void* (*ASSET_MANAGER_OPEN)(void* target, const char* fileName, AccessMode mode);

Elf_Addr gCallOrg = 0;
Elf_Addr gCallOrgFread = 0;
Elf_Addr gCallOrgOpenNonAsset = 0;
Elf_Addr gCallOrgUncon = 0;
Elf_Addr gCallOrgCreateEntry = 0;
Elf_Addr gCallOrgAssetFileRead = 0;

FAInHook* gHook = NULL;
FAInHook* gHookMap = NULL;

char* gPath = NULL;

AAsset* my_AAssetManager_open(AAssetManager* mgr, const char* filename, int mode);
void* my_am_open(AAssetManager* pObj, const char* fileName, AccessMode mode);


extern "C" JNIEXPORT jstring JNICALL
Java_com_yyzhu_reinforceres_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved){
    JNIEnv* jenv = NULL;
    vm->GetEnv((void**)&jenv, JNI_VERSION_1_4);


    if(1){
        gHook = FAInHook::instance();
        auto libc = dlopen("libc.so", RTLD_NOW);
        auto libandroidfw = dlopen("libandroid.so", RTLD_NOW);//on original emu can not find libandroidfw.so
        auto libutils = dlopen("libutils.so", RTLD_NOW);
        auto libandroidrt = dlopen("libandroid_runtime.so", RTLD_NOW);
        //solve cast error hook c++ method!

        //char buf[12];
        //sprintf(buf,"%u", &AssetManager::myopen);
        //int methodAddr = atoi(buf); //这里为什么错呢？后面验证发现其实atoi算出来的结果是对的，但是转为int的时候出错了！
        //int methodAddr = *(int*)buf;这样直接不行，因为buf中其实是地址的字符串
        //Elf_Addr methodAddr = (Elf_Addr)atof(buf);//直接将结果转为Elf_Addr就行了，估计是int大小不够

        char* sym1 = "_ZN7android12AssetManager24openNonAssetInPathLockedEPKcNS_5Asset10AccessModeERKNS0_10asset_pathE";
        Elf_Addr addr_target = (Elf_Addr)dlsym(libandroidfw, sym1);
        Elf_Addr addr_hook = (Elf_Addr)openNonAssetInPathLocked;


/*        char* sym2 = "_ZNK7android9ZipFileRO18createEntryFileMapEPv";
        Elf_Addr addr_target2 = (Elf_Addr)dlsym(libandroidfw, sym2);
        Elf_Addr addr_hook2 = (Elf_Addr)createEntryFileMap2;*/

/*        char* sym3 = "_ZN7android7FileMapD2Ev";
        Elf_Addr addr_target3 = (Elf_Addr)dlsym(libandroidfw, sym3);
        Elf_Addr addr_hook3 = (Elf_Addr)unconstructor_FileMap;*/

        char* sym4 = "fread";
        Elf_Addr addr_target4 = (Elf_Addr)dlsym(libc, sym4);
        Elf_Addr addr_hook4 = (Elf_Addr)myfread;


        char* sym5 = "_ZN7android10_FileAsset4readEPvj";
        Elf_Addr addr_target5 = (Elf_Addr)dlsym(libandroidfw, sym5);
        Elf_Addr addr_hook5 = (Elf_Addr)fileAssetRead;

        if(gHook->registerHook(addr_target, addr_hook, &gCallOrgOpenNonAsset) != FAInHook::FERROR_SUCCESS){
            LOGD("hook %s fail!", sym1);
        }

/*        if(gHook->registerHook(addr_target2, addr_hook2, &gCallOrgCreateEntry) != FAInHook::FERROR_SUCCESS){
            LOGD("hook %s fail!", sym2);
        }*/

/*        if(gHook->registerHook(addr_target3, addr_hook3, &gCallOrgUncon) != FAInHook::FERROR_SUCCESS){
            LOGD("hook %s fail!", sym3);
        }*/

        if(gHook->registerHook(addr_target4, addr_hook4, &gCallOrgFread) != FAInHook::FERROR_SUCCESS){
            LOGD("hook %s fail!", sym4);
        }

        if(gHook->registerHook(addr_target5, addr_hook5, &gCallOrgAssetFileRead) != FAInHook::FERROR_SUCCESS){
            LOGD("hook %s fail!", sym5);
        }

        gHook->hookAll();
    }

    //获取base.apk的路径
    jobject application = getApplication(jenv);
    jstring path = getApkPath(jenv, application);
    gPath = jstringToChar(jenv, path);


    return JNI_VERSION_1_4;
}

typedef size_t (*MYFREAD)(void *buffer, size_t size, size_t count, FILE * stream);
size_t  myfread(void *buffer, size_t size, size_t count, FILE * stream){

    LOGD("invoke myfread");
    int fd = fileno(stream);
    LOGD("fd is %d", fd);
    char tmp[1024] = {'\0'};
    char file_path[1024] = {'\0'};
    snprintf(tmp,sizeof(tmp), "/proc/self/fd/%d", fd);
    readlink(tmp,file_path,sizeof(file_path)-1);
    LOGD("file path is %s", file_path);


    size_t ret = 0;
    MYFREAD pFunc = (MYFREAD)gCallOrgFread;
    ret = pFunc(buffer,size,count,stream);

    if(NULL != strstr(file_path, "mybase.apk")){
        if(0 == memcmp(buffer, "hello world!", ret)){
            memcpy(buffer, "yyzhu zhuyy!", 12);
        }
    }

    return ret;
}


FILE* myfopen(const char *path, const char *mode){
    LOGD("invoke myfopen");
    LOGD("path: %s", path);
    LOGD("invoke org fopen");
    FILE* ret = NULL;

    ret = ((FOPEN)gCallOrg)(path, mode);

    LOGD("org fopen return value is %p", ret);
    return ret;
}


int myopen(const char *path, int oflags, mode_t mode){
    LOGD("invoke myopen");
    LOGD("path: %s", path);
    LOGD("oflags is %d", oflags);
    LOGD("mode is %d", mode);

    gHook->unhookAll();

    LOGD("invoke org open");
    int ret = -1;

    //ret = ((OPEN)gCallOrg)(path, oflags, mode);
    ret = open(path, oflags, mode);


    //hook again
    auto libc = dlopen("libc.so", RTLD_NOW);

    Elf_Addr addr_target = (Elf_Addr)dlsym(libc, "open");
    Elf_Addr addr_hook = (Elf_Addr)myopen;

    gHook->registerHook(addr_target, addr_hook, &gCallOrg);

    gHook->hookAll();

    LOGD("org open return value is %d", ret);
    return ret;
}


int myopen2(const char *path, int oflags){
    LOGD("invoke myopen2");
    LOGD("path: %s", path);
    LOGD("oflags is %d", oflags);


    gHook->unhookAll();

    LOGD("invoke org open");
    int ret = -1;

    //ret = ((OPEN)gCallOrg)(path, oflags, mode);
    ret = open(path, oflags); //这里其实应该调用__open_2但是无奈是内部函数，无法直接在ide中编码调用，但好在参数是一样的，内部只有分发操作


    //hook again
    auto libc = dlopen("libc.so", RTLD_NOW);
    Elf_Addr addr_target = (Elf_Addr)dlsym(libc, "__open_2");
    Elf_Addr addr_hook = (Elf_Addr)myopen2;

    gHook->registerHook(addr_target, addr_hook, &gCallOrg);

    gHook->hookAll();

    LOGD("org open return value is %d", ret);
    return ret;
}

AAsset* my_AAssetManager_open(AAssetManager* mgr, const char* filename, int mode){
    LOGD("invoke my_AAssetManager_open");
    LOGD("filename is %s", filename);
    LOGD("mode is %d", mode);

    gHook->unhookAll();
    LOGD("invoke org AAssetManager_open");
    AAsset* asset = NULL;
    asset = AAssetManager_open(mgr, filename, mode);
    LOGD("ret is %p", asset);

    auto libandroid = dlopen("libandroid.so", RTLD_NOW);
    Elf_Addr addr_target = (Elf_Addr)dlsym(libandroid, "AAssetManager_open");
    Elf_Addr addr_hook = (Elf_Addr)my_AAssetManager_open;

    gHook->registerHook(addr_target, addr_hook, &gCallOrg);

    gHook->hookAll();

    return asset;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yyzhu_reinforceres_MainActivity_clickOpenAsset(JNIEnv *env, jobject thiz) {
    // TODO: implement clickOpenAsset()

    LOGD("invoke clickOpenFile");

    AAssetManager* mgr;
    jclass cls = env->GetObjectClass(thiz);
    jmethodID mid = env->GetMethodID(cls, "getAssets","()Landroid/content/res/AssetManager;");
    jobject jobj_assetManager = env->CallObjectMethod(thiz, mid);
    mgr = AAssetManager_fromJava(env, jobj_assetManager);
    if(mgr == NULL){
        LOGD("AAssetManager is NULL!");
        return;
    }

    AAsset* as = AAssetManager_open(mgr, "reinforceRes.txt", AASSET_MODE_STREAMING);


    int lenth = AAsset_getLength(as);

    LOGD("file lenth is %d", lenth);

    char* pBuff = new char[lenth + 1];
    memset(pBuff, 0, lenth+1);

    int nBytesRead = AAsset_read(as, pBuff, lenth);

    LOGD("file content: %s", pBuff);

    AAsset_close(as);

    delete[] pBuff;
}

typedef size_t (*MYREAD)(int fd, void *buf, size_t count);
size_t myread(int fd, void *buf, size_t count){

    LOGD("invoke myread");
    LOGD("fd is %d", fd);
    char tmp[1024] = {'\0'};
    char file_path[1024] = {'\0'};
    snprintf(tmp,sizeof(tmp), "/proc/self/fd/%d", fd);
    int nn = readlink(tmp,file_path,sizeof(file_path)-1);

    LOGD("file path is %s", file_path);


    size_t ret = 0;
    MYREAD pFunc = (MYREAD)gCallOrgFread;
    ret = pFunc(fd, buf, count);

    if(NULL != strstr(file_path, "mybase.apk")){
        if(0 == memcmp(buf, "hello world!", ret)){
            memcpy(buf, "yyzhu zhuyy!", 12);
        }
    }

    return ret;

}


Asset* AssetManager::myopen(const char* fileName, AccessMode mode){
    LOGD("invoke native AssetManager::myopen");
    LOGD("filename is %s", fileName);

    gHook->unhookAll();
    //这个函数结尾会崩溃，后面证明这里的AAssetManager_open与AssetManager::open不等价，还是要通过原地址调用
    AAsset* ret = AAssetManager_open((AAssetManager*)this, fileName, mode);
    LOGD("AAsset is %p", ret);

    auto libandroidfw = dlopen("libandroid.so", RTLD_NOW);

    //solve cast error!
    char buf[12] = {0};
    sprintf(buf,"%u", &AssetManager::myopen);
    //int methodAddr = atoi(buf); //这里为什么错呢？后面验证发现其实atoi算出来的结果是对的，但是转为int的时候出错了！
    //int methodAddr = *(int*)buf;这样直接不行，因为buf中其实是地址的字符串
    Elf_Addr methodAddr = (Elf_Addr)atof(buf);//直接将结果转为Elf_Addr就行了，估计是int大小不够
    Elf_Addr addr_target = (Elf_Addr)dlsym(libandroidfw, "_ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE");
    Elf_Addr addr_hook = (Elf_Addr)methodAddr;


    gHook->registerHook(addr_target, addr_hook, &gCallOrg);

    gHook->hookAll();

    Asset* rret = (Asset*)ret;
    return rret;
}


//_ZN7android12AssetManager4openEPKcNS_5Asset10AccessModeE
void* my_am_open(AAssetManager* pObj, const char* fileName, AccessMode mode){
    LOGD("invoke native my_am_open");
    LOGD("filename is %s", fileName);

    //gHook->unhookAll();
    ASSET_MANAGER_OPEN f = (ASSET_MANAGER_OPEN)gCallOrg;

    char* newfilename = "/sdcard/1.txt";
    LOGD("replace filename with \"/sdcard/1.txt\"");
    void* ret = f(pObj, newfilename, mode);
    LOGD("Asset is %p", ret);

    return ret;
}

//Asset* Asset::createFromCompressedFile(const char* fileName, AccessMode mode)
//_ZN7android5Asset24createFromCompressedFileEPKcNS0_10AccessModeE
void* my_createFromCompressedFile(Asset* pObject, const char* fileName, AccessMode mode){
    LOGD("invoke native my_createFromCompressedFile");
    LOGD("filename is %s", fileName);

    ASSET_MANAGER_OPEN f = (ASSET_MANAGER_OPEN)gCallOrg;
    void* ret = f(pObject, fileName, mode);
    LOGD("Asset is %p", ret);

    return ret;

}

//_ZN7android5Asset14createFromFileEPKcNS0_10AccessModeE
//Asset* Asset::createFromFile(const char* fileName, AccessMode mode)
void* my_createFromFile(Asset* pObject, const char* fileName, AccessMode mode){
    LOGD("invoke native my_createFromCompressedFile");
    LOGD("filename is %s", fileName);

    ASSET_MANAGER_OPEN f = (ASSET_MANAGER_OPEN)gCallOrg;
    void* ret = f(pObject, fileName, mode);
    LOGD("Asset is %p", ret);

    return ret;
}


typedef Asset* (*CREATFROMCOMPRESSEDMAP)(void* mythis, void* dataMap,  size_t uncompressedLen, AccessMode mode);

Asset* createFromCompressedMap(void* mythis, void* dataMap,  size_t uncompressedLen, AccessMode mode){

    return ((CREATFROMCOMPRESSEDMAP)gCallOrg)(mythis, dataMap, uncompressedLen, mode);
}


// FileMap* ZipFileRO::createEntryFileMap(ZipEntryRO entry) const
typedef void* (*CREATE_ENTRY_FILE_MAP)(void* pThis, void* entry);
void* createEntryFileMap(void* pThis, void* entry){

    LOGD("invoke native hooked createEntryFileMap");

    _ZipEntryRO* pZipEntryR0 = (_ZipEntryRO*)entry;
    ZipString zStr= pZipEntryR0->name;
    char* assetName = (char*)zStr.name;

    if( strstr("assets/reinforceRes.txt", assetName) ){
        auto libandroidfw = dlopen("libandroidfw.so", RTLD_NOW);
        Elf_Addr addr_target = (Elf_Addr)dlsym(libandroidfw, "_ZNK7android9ZipFileRO18createEntryFileMapEPv");

        My_ZipEntry &ze = pZipEntryR0->entry;
        int fd = open(gPath, O_RDONLY);//这个文件只能以只读方式打开，不然返回-1
        size_t actualLen = 0;
        if (ze.method == 0) {
            actualLen = ze.uncompressed_length;
        } else {
            actualLen = ze.compressed_length;
        }

        FileMap *newMap = new FileMap();
        if (!newMap->create(gPath, fd, ze.offset, actualLen, true)) {
            delete newMap;
            return NULL;
        }

        void* pData = newMap->getDataPtr();
        int nDataLenth = newMap->getDataLength();
        //memcpy(pData,"yyzhu", 5); 直接修改映射的内存会触发访问异常

        char* pBuff = new char[nDataLenth];
        memset(pBuff, 0, nDataLenth);
        memcpy(pBuff, pData, nDataLenth);
        memcpy(pBuff, "yyzhu", 5);
        //这里修改了之后，要在hook的析构函数里面处理，但是由于这个分支不一定走，所以这里的mDataPtr有可能是原来的map中的一个位置，所以析构的时候要判断这个位置
        //是否在map范围中,如果在范围中的话就直接调用原析构函数，不然delete掉mDataPtr，范围可以通过mBasePtr和mBaseLength判断。
        newMap->mDataPtr = pBuff;

        close(fd);

        return newMap;
    }

    CREATE_ENTRY_FILE_MAP orgfunc = (CREATE_ENTRY_FILE_MAP)gCallOrgCreateEntry;


    return orgfunc(pThis, entry);
}


void* createEntryFileMap2(void* pThis, void* entry){

    LOGD("invoke native hooked createEntryFileMap");

    _ZipEntryRO* pZipEntryR0 = (_ZipEntryRO*)entry;
    ZipString zStr= pZipEntryR0->name;
    char* assetName = (char*)zStr.name;

    if( strstr("assets/reinforceRes.txt", assetName) ){
        if(false){
            auto libandroidfw = dlopen("libandroidfw.so", RTLD_NOW);
            Elf_Addr addr_target = (Elf_Addr)dlsym(libandroidfw, "_ZNK7android9ZipFileRO18createEntryFileMapEPv");

            My_ZipEntry &ze = pZipEntryR0->entry;

            char apkname[1024]={0};
            strcpy(apkname, gPath);
            int nlenth = strlen(apkname);
            strcpy((char*)((int)apkname+nlenth-8), "mybase.apk");

            int fd = open(apkname, O_RDWR);//这个文件只能以只读方式打开，不然返回-1
            size_t actualLen = 0;
            if (ze.method == 0) {
                actualLen = ze.uncompressed_length;
            } else {
                actualLen = ze.compressed_length;
            }

            FileMap *newMap = new FileMap();
            if (!newMap->create(gPath, fd, ze.offset, actualLen, false)) {
                delete newMap;
                return NULL;
            }

            void* pData = newMap->getDataPtr();
            int nDataLenth = newMap->getDataLength();

            for(int i=0;i<nDataLenth;i++){
                ((char*)pData)[i] = 'z';
            }

            close(fd);

            return newMap;
        }

        return NULL;

    }

    CREATE_ENTRY_FILE_MAP orgfunc = (CREATE_ENTRY_FILE_MAP)gCallOrgCreateEntry;

    return orgfunc(pThis, entry);
}



//这里修改了之后，要在hook的析构函数里面处理，但是由于这个分支不一定走，所以这里的mDataPtr有可能是原来的map中的一个位置，所以析构的时候要判断这个位置
//是否在map范围中,如果在范围中的话就直接调用原析构函数，不然delete掉mDataPtr，范围可以通过mBasePtr和mBaseLength判断。
typedef void (*UNCONSTRUCTOR_FILEMAP)(void* pThis);
void unconstructor_FileMap(void* pThis){

    LOGD("invoke native hooked unconstructor_FileMap");

    FileMap* pFileMap = (FileMap*)pThis;
    void* pData = pFileMap->mDataPtr;
    int sky = (int)pFileMap->mBasePtr + (int)pFileMap->mBaseLength;
    int groud = (int)pFileMap->mBasePtr;
    bool handle = true;

    if(pData == NULL ||
            (((int)pData >= groud) && ((int)pData <= sky))
            ){
        handle = false;
    }

    if(handle){
        LOGD("unconstructor_FileMap delete FileMap::mDataPtr");
        delete pData;
    }

    UNCONSTRUCTOR_FILEMAP org = (UNCONSTRUCTOR_FILEMAP)gCallOrgUncon;
    org(pThis);
}

typedef void* (*OPENNONASSETINPATHLOCKED)(void* pThis, const char* fileName, AccessMode mode, const asset_path& path);
void* openNonAssetInPathLocked(void* pThis, const char* fileName, AccessMode mode, const asset_path& path){
    if(NULL != strstr(fileName, "assets/reinforceRes.txt")){
        asset_path newpath = asset_path();
        char apkname[100]={0};
        strcpy(apkname, gPath);
        int nlenth = strlen(apkname);
        strcpy((char*)((int)apkname+nlenth-8), "mybase.apk");

        //newpath.path.mString = "/sdcard/mybase.apk";
        //newpath.path.mString = path.path.mString;
        // "/data/data/com.yyzhu.reinforceres/files/mybase.apk";这个地址在java层修改可读权限也会发生议程
        newpath.path.mString = apkname;
        OPENNONASSETINPATHLOCKED pFunc = (OPENNONASSETINPATHLOCKED)gCallOrgOpenNonAsset;
        return pFunc(pThis, fileName, mode, newpath);
    }

    OPENNONASSETINPATHLOCKED pFunc = (OPENNONASSETINPATHLOCKED)gCallOrgOpenNonAsset;
    return pFunc(pThis, fileName, mode, path);
}

typedef ssize_t (*FILEASSETREAD)(_FileAsset* pThis, void* buf, size_t count);
ssize_t fileAssetRead(_FileAsset* pThis, void* buf, size_t count){

    char* targetFilename = "mybase.apk";
    bool isTargetFile = false;
    if(pThis->mMap != NULL){
        char * filename = pThis->mMap->mFileName;
        if(strstr(filename, targetFilename) != NULL){isTargetFile = true;}
    }else{

    }

    if(isTargetFile){
        return pThis->read(buf, count); //这里调用的是我自己的修改过的_FileAsset的方法，这个方法一定不能设置成虚函数，因为对象不是我的代码new出来的，虚表和我不一致
    }else{
        FILEASSETREAD pFunc = (FILEASSETREAD)gCallOrgAssetFileRead;
        return pFunc(pThis, buf, count);
    }

}