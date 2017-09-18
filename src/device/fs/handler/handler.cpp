//
// Created by 王耀 on 2017/9/16.
//

#include "handler.h"

Handler::Handler(const std::string &path) {
    mPath = path;
}

bool Handler::init(unsigned long bufferSize) {
    mBufferSize = bufferSize;
    mUsedSize = 0;
    mBlockList = nullptr;
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
