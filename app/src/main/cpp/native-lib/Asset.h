/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Class providing access to a read-only asset.  Asset objects are NOT
// thread-safe, and should not be shared across threads.
//
#ifndef __LIBS_ASSET_H
#define __LIBS_ASSET_H

#include <stdio.h>
#include <sys/types.h>

#include "FileMap.h"
#include "androiddef.h"

/*
 * Instances of this class provide read-only operations on a byte stream.
 *
 * Access may be optimized for streaming, random, or whole buffer modes.  All
 * operations are supported regardless of how the file was opened, but some
 * things will be less efficient.  [pass that in??]
 *
 * "Asset" is the base class for all types of assets.  The classes below
 * provide most of the implementation.  The AssetManager uses one of the
 * static "create" functions defined here to create a new instance.
 */

typedef enum AccessMode {
    ACCESS_UNKNOWN = 0,

    /* read chunks, and seek forward and backward */
            ACCESS_RANDOM,

    /* read sequentially, with an occasional forward seek */
            ACCESS_STREAMING,

    /* caller plans to ask for a read-only buffer with all data */
            ACCESS_BUFFER,
} AccessMode;


class Asset {
public:
    virtual ~Asset(void) = default;


public:
    AccessMode  mAccessMode;        // how the asset was opened
    String8    mAssetSource;       // debug string
    
    Asset*		mNext;				// linked list.
    Asset*		mPrev;
};


class _FileAsset : public Asset {
public:
    _FileAsset(void);
    virtual ~_FileAsset(void);

    ssize_t read(void* buf, size_t count);
    const void* getBuffer(bool wordAligned);

    const void* ensureAlignment(FileMap* map);

public:
    off64_t     mStart;         // absolute file offset of start of chunk
    off64_t     mLength;        // length of the chunk
    off64_t     mOffset;        // current local offset, 0 == mStart
    FILE*       mFp;            // for read/seek
    char*       mFileName;      // for opening

    /*
     * To support getBuffer() we either need to read the entire thing into
     * a buffer or memory-map it.  For small files it's probably best to
     * just read them in.
     */
    enum { kReadVsMapThreshold = 4096 };

    FileMap*    mMap;           // for memory map
    unsigned char* mBuf;        // for read

};


class AssetManager{
public:
    Asset* myopen(const char* fileName, AccessMode mode);
    //Asset* open(const char* fileName, AccessMode mode);
};

#endif // __LIBS_ASSET_H
