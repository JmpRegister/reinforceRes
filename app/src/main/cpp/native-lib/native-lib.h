//
// Created by thr0wCat on 2019-11-06.
//

#ifndef REINFORCERES_NATIVE_LIB_H
#define REINFORCERES_NATIVE_LIB_H
#include <cstdlib>
#include "Asset.h"
#include "androiddef.h"

FILE* myfopen(const char *path, const char *mode);

int myopen(const char *path, int oflags, mode_t mode);

int myopen2(const char *path, int oflags);

size_t myread(int fd, void *buf, size_t count);

size_t  myfread(void *buffer, size_t size, size_t count, FILE * stream);

void* createEntryFileMap(void* pThis, void* entry);

void* createEntryFileMap2(void* pThis, void* entry);

void unconstructor_FileMap(void* pThis);

void* my_createFromCompressedFile(Asset* pObject, const char* fileName, AccessMode mode);

void* my_createFromFile(Asset* pObject, const char* fileName, AccessMode mode);

Asset* createFromCompressedMap(void* mythis, void* dataMap,  size_t uncompressedLen, AccessMode mode);

void* openNonAssetInPathLocked(void* pThis, const char* fileName, AccessMode mode, const asset_path& path);

ssize_t fileAssetRead(_FileAsset* pThis, void* buf, size_t count);




#endif //REINFORCERES_NATIVE_LIB_H
