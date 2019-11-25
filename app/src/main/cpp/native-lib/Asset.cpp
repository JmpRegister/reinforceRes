#include "Asset.h"
#include "FileMap.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>


/*
 * Destructor.  Release resources.
 */
_FileAsset::~_FileAsset(void)
{

}

/*
 * Read a chunk of data.
 */
ssize_t _FileAsset::read(void* buf, size_t count)
{
    size_t maxLen;
    size_t actual;

    assert(mOffset >= 0 && mOffset <= mLength);

    if (mAccessMode == ACCESS_BUFFER) {
        /*
         * On first access, read or map the entire file.  The caller has
         * requested buffer access, either because they're going to be
         * using the buffer or because what they're doing has appropriate
         * performance needs and access patterns.
         */
        if (mBuf == NULL)
            getBuffer(false);
    }

    /* adjust count if we're near EOF */
    maxLen = mLength - mOffset;
    if (count > maxLen)
        count = maxLen;

    if (!count)
        return 0;

    if (mMap != NULL) {
        /* copy from mapped area */
        //printf("map read\n");
        memcpy(buf, (char*)mMap->getDataPtr() + mOffset, count);

        actual = count;
    } else if (mBuf != NULL) {
        /* copy from buffer */
        //printf("buf read\n");
        memcpy(buf, (char*)mBuf + mOffset, count);
        actual = count;
    } else {
        /* read from the file */
        //printf("file read\n");
        if (ftell(mFp) != mStart + mOffset) {
            assert(false);
        }

        /*
         * This returns 0 on error or eof.  We need to use ferror() or feof()
         * to tell the difference, but we don't currently have those on the
         * device.  However, we know how much data is *supposed* to be in the
         * file, so if we don't read the full amount we know something is
         * hosed.
         */
        actual = fread(buf, 1, count, mFp);
        if (actual == 0)        // something failed -- I/O error?
            return -1;

        assert(actual == count);
    }

    mOffset += actual;

    //myadd,后续在这里做解密操作
    if (memcmp(buf, "hello world!", count) == 0) {
        for (int i = 0; i < count; i++) {
            ((char *) buf)[i] = 'Z';
        }
    }

    return actual;
}


/*
 * Return a read-only pointer to a buffer.
 *
 * We can either read the whole thing in or map the relevant piece of
 * the source file.  Ideally a map would be established at a higher
 * level and we'd be using a different object, but we didn't, so we
 * deal with it here.
 */
const void* _FileAsset::getBuffer(bool wordAligned)
{
    /* subsequent requests just use what we did previously */
    if (mBuf != NULL)
        return mBuf;
    if (mMap != NULL) {
        if (!wordAligned) {
            return  mMap->getDataPtr();
        }
        return ensureAlignment(mMap);
    }

    assert(mFp != NULL);

    if (mLength < kReadVsMapThreshold) {
        unsigned char* buf;
        long allocLen;

        /* zero-length files are allowed; not sure about zero-len allocs */
        /* (works fine with gcc + x86linux) */
        allocLen = mLength;
        if (mLength == 0)
            allocLen = 1;

        buf = new unsigned char[allocLen];
        if (buf == NULL) {
            return NULL;
        }

        if (mLength > 0) {
            long oldPosn = ftell(mFp);
            fseek(mFp, mStart, SEEK_SET);
            if (fread(buf, 1, mLength, mFp) != (size_t) mLength) {
                delete[] buf;
                return NULL;
            }
            fseek(mFp, oldPosn, SEEK_SET);
        }

        mBuf = buf;
        return mBuf;
    } else {
        FileMap* map;

        map = new FileMap;
        if (!map->create(NULL, fileno(mFp), mStart, mLength, true)) {
            delete map;
            return NULL;
        }


        mMap = map;
        if (!wordAligned) {
            return  mMap->getDataPtr();
        }
        return ensureAlignment(mMap);
    }
}


const void* _FileAsset::ensureAlignment(FileMap* map)
{
    void* data = map->getDataPtr();
    if ((((size_t)data)&0x3) == 0) {
        // We can return this directly if it is aligned on a word
        // boundary.
        return data;
    }
    // If not aligned on a word boundary, then we need to copy it into
    // our own buffer.

    unsigned char* buf = new unsigned char[mLength];
    if (buf == NULL) {
        return NULL;
    }
    memcpy(buf, data, mLength);
    mBuf = buf;
    return buf;
}
