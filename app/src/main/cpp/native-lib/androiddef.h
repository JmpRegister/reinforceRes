//
// Created by thr0wCat on 2019-11-22.
//

#ifndef REINFORCERES_ANDROIDDEF_H
#define REINFORCERES_ANDROIDDEF_H


#include "Asset.h"

class String8{
public:
    String8(){
        mString = "";
    }
    const char* mString;
};

typedef enum FileType {
    kFileTypeUnknown = 0,
    kFileTypeNonexistent,       // i.e. ENOENT
    kFileTypeRegular,
    kFileTypeDirectory,
    kFileTypeCharDev,
    kFileTypeBlockDev,
    kFileTypeFifo,
    kFileTypeSymlink,
    kFileTypeSocket,
} FileType;

struct asset_path {
public:
    asset_path() :type(kFileTypeRegular),
                  isSystemOverlay(false), isSystemAsset(false) {
    }


    String8 path;
    FileType type;
    String8 idmap;
    bool isSystemOverlay;
    bool isSystemAsset;
};



#endif //REINFORCERES_ANDROIDDEF_H
