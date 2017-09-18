//
// Created by 王耀 on 2017/9/16.
//

#ifndef SDOS_HANDLER_H
#define SDOS_HANDLER_H

#include <cstdio>
#include <string>

class Handler {
private:
    struct BlockInfo {
        long startPos;
        int blockCount;
        BlockInfo * next;
    };
    BlockInfo * mBlockList;
    long mCurrentPos;
    long mBufferSize;
    long mUsedSize;
    char * mBuffer;
    std::string mPath;
public:
    explicit Handler(const std::string &path);
    bool init(unsigned long bufferSize);
    bool reset();
    bool seek(long pos);
    long write(char *src, long size);
    long read(char *dst, long size);
};

#endif //SDOS_HANDLER_H
