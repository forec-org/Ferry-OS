//
// Created by 王耀 on 2017/9/16.
//

#include "../../../../include/device/fs/fs.h"
#include "../../../../include/device/fs/handler/handler.h"

Handler::Handler(const std::string &path, FS* parent) {
    mPath = path;
    mParent = parent;
}

bool Handler::init(long basePos, char *buffer, long bufferSize) {
// TODO
    return true;
}

bool Handler::reset() {
// TODO
    return true;
}

bool Handler::seek(long pos) {
// TODO
    return true;
}

long Handler::read(char *dst, long size) {
// TODO
    return 0;
}

long Handler::write(char *src, long size) {
// TODO
    return 0;
}
