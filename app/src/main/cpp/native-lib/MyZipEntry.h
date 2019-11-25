//
// Created by thr0wCat on 2019-11-20.
//

#ifndef REINFORCERES_MYZIPENTRY_H
#define REINFORCERES_MYZIPENTRY_H

class ZipEntry {
public:
    ~ZipEntry(void) {}

    bool mDeleted;       // set if entry is pending deletion
    bool mMarked;        // app-defined marker

    class LocalFileHeader {
    public:
        virtual ~LocalFileHeader(void) {}
        // unsigned long mSignature;
        unsigned short mVersionToExtract;
        unsigned short mGPBitFlag;
        unsigned short mCompressionMethod;
        unsigned short mLastModFileTime;
        unsigned short mLastModFileDate;
        unsigned long mCRC32;
        unsigned long mCompressedSize;
        unsigned long mUncompressedSize;
        unsigned short mFileNameLength;
        unsigned short mExtraFieldLength;
        unsigned char *mFileName;
        unsigned char *mExtraField;
    };

    class CentralDirEntry {
    public:
        virtual ~CentralDirEntry(void){}
        // unsigned long mSignature;
        unsigned short mVersionMadeBy;
        unsigned short mVersionToExtract;
        unsigned short mGPBitFlag;
        unsigned short mCompressionMethod;
        unsigned short mLastModFileTime;
        unsigned short mLastModFileDate;
        unsigned long mCRC32;
        unsigned long mCompressedSize;
        unsigned long mUncompressedSize;
        unsigned short mFileNameLength;
        unsigned short mExtraFieldLength;
        unsigned short mFileCommentLength;
        unsigned short mDiskNumberStart;
        unsigned short mInternalAttrs;
        unsigned long mExternalAttrs;
        unsigned long mLocalHeaderRelOffset;
        unsigned char *mFileName;
        unsigned char *mExtraField;
        unsigned char *mFileComment;

    };

    LocalFileHeader mLFH;
    CentralDirEntry mCDE;
};

struct ZipString {
 const uint8_t* name;
 uint16_t name_length;
};

struct  My_ZipEntry{
        uint16_t method;

        uint32_t mod_time;

        uint8_t has_data_descriptor;

        uint32_t crc32;

        uint32_t compressed_length;

        uint32_t uncompressed_length;

        off64_t offset;
};

class _ZipEntryRO {
public:
    My_ZipEntry entry;
    ZipString name;
    void *cookie;

    ~_ZipEntryRO() {}

};


#endif REINFORCERES_MYZIPENTRY_H
