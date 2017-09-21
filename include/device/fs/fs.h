//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_FS_H
#define SDOS_FS_H

#include "../../config/macro.h"
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

class FS {
private:
    boost::filesystem::path mBasePath;
    std::string mSwapPath;
    unsigned long mSwapUsedPage;
    unsigned long mSwapAllocPage;
    static unsigned long mSwapSpacePage;
    std::vector<bool> mBitmap;
    std::vector<bool> mAllocmap;
    FS(const std::string& path = "./fs");
    ~FS();
    static FS * instance;
public:
    static void init(const std::string &path = "./fs", unsigned long swapSpaceSize = _OS_MEM_SWAP_PAGE);
    static void destroy();
    static void format();
    static FS * getInstance();

    std::string getBasePath();
    std::string getPath(const std::string &path);
    unsigned long getSwapUsedPage();
    unsigned long getSwapSpacePage();

    bool isDirectory(const std::string &path);
    bool isFile(const std::string &path);
    bool isExist(const std::string &path);
    bool rmFile(const std::string &path);
    bool cpFile(const std::string &path, const std::string &newpath);
    bool mvFile(const std::string &path, const std::string &newpath);
    bool mkdir(const std::string &path);
    bool rmdir(const std::string &path);
    bool touchFile(const std::string &path);

    unsigned long getFileSize(const std::string &path);
    char readFileByte(const std::string &path, unsigned long offset = 0);
    bool readFile(const std::string& path, const void * dst, unsigned long size, unsigned long start = 0);
    bool writeFile(const std::string& path, const void * src, unsigned long size, unsigned long start = 0);
    bool appendFile(const std::string& path, const void * src, unsigned long size);

    unsigned long allocSwapAddress();
    void clearSwapPage(unsigned long swapPage);
    bool dumpPageIntoSwap(const void * src, unsigned long swapPages);
    bool loadPageIntoMemory(const void * dst, unsigned long swapPage);
};

#endif //SDOS_FS_H
