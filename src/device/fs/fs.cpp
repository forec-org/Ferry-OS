//
// Created by 王耀 on 2017/9/21.
//

#include "config.h"
#include <iostream>
#include <fstream>
#include "fs.h"

using namespace std;
using namespace boost::filesystem;

unsigned long FS::mSwapSpacePage = 0;
FS *FS::instance = nullptr;

void FS::init(const std::string &path, unsigned long swapSpacePage) {
    mSwapSpacePage = swapSpacePage;

    if (!exists(path)) {
        create_directories(path);
    }
    try {
        instance = new FS(path);
    } catch(std::bad_alloc &e) {
        std::cerr << "Alloc FS Error: " << e.what() << std::endl;
        return;
    }

    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    instance->rubbish = new char[PAGE];
    memset(instance->rubbish, 0, PAGE);
}

void FS::destroy() {
    delete instance;
}

FS *FS::getInstance() {
    if (!instance)
        FS::init();
    return instance;
}

bool FS::isExist(const std::string &path) {
    return exists(getPath(path));
}

bool FS::mkdir(const std::string &path) {
    return exists(getPath(path)) || create_directories(getPath(path));
}

bool FS::rmdir(const std::string &path) {
    if (!exists(getPath(path)))
        return false;
    return remove_all(getPath(path)) > 0;
}

FS::FS(const std::string &path) {
    mBasePath = boost::filesystem::path(path);
    mSwapPath = ".swap";
    touchFile(mSwapPath);
    mSwapUsedPage = 0;
    mSwapAllocPage = 0;
    mBitmap.clear();
    mAllocmap.clear();
    mBitmap.resize(FS::mSwapSpacePage, false);
    mAllocmap.resize(FS::mSwapSpacePage, false);
}

FS::~FS() {
    delete rubbish;
    remove(getPath(mSwapPath));
}

std::string FS::getBasePath() {
    return std::string(mBasePath.c_str());
}

std::string FS::getPath(const std::string &path) {
    boost::filesystem::path npath = mBasePath;
    npath.append(path);
    return std::string(npath.c_str());
}

unsigned long FS::getSwapFileSize() {
    if (!isExist(mSwapPath))
        return 0;
    return getFileSize(mSwapPath);
}

bool FS::isDirectory(const std::string &path) {
    return is_directory(getPath(path));
}

bool FS::isFile(const std::string &path) {
    return is_regular_file(getPath(path));
}

bool FS::touchFile(const std::string &path) {
    return writeFile(path, nullptr, 0);
}

bool FS::rmFile(const std::string &path) {
    return remove(getPath(path));
}

bool FS::cpFile(const std::string &path, const std::string &newpath) {
    if (!isExist(path))
        return false;
    try {
        copy_file(getPath(path), getPath(newpath), copy_option::overwrite_if_exists);
    } catch (filesystem_error &e) {
        std::cerr << "Copy Error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool FS::mvFile(const std::string &path, const std::string &newpath) {
    if (!isExist(path))
        return false;
    try {
        copy_file(getPath(path), getPath(newpath), copy_option::overwrite_if_exists);
    } catch (filesystem_error &e) {
        std::cerr << "Rename Error: " << e.what() << std::endl;
        return false;
    }
    rmFile(path);
    return true;
}

unsigned long FS::getSwapUsedPage() {
    return mSwapUsedPage;
}

unsigned long FS::getSwapAllocedPage() {
    return mSwapAllocPage;
}

unsigned long FS::getSwapSpacePage() {
    return FS::mSwapSpacePage;
}

unsigned long FS::allocSwapPage() {
    if (mSwapUsedPage < mSwapAllocPage) {
        for (unsigned long i = 0; i < mSwapAllocPage; i++) {
            if (mAllocmap[i] && !mBitmap[i]) {
                mSwapUsedPage++;
                mBitmap[i] = true;
                return i;
            }
        }
    }
    if (mSwapAllocPage >= mSwapSpacePage)
        return mSwapSpacePage;
    unsigned long swapPage = mSwapAllocPage;
    mBitmap[mSwapAllocPage] = true;
    mAllocmap[mSwapAllocPage] = true;
    mSwapUsedPage++;
    mSwapAllocPage++;
    dumpRubbish(swapPage);
    return swapPage;
}

void FS::dumpRubbish(unsigned long swapPage) {
    dumpPageIntoSwap(rubbish, swapPage);
}

void FS::clearSwapPage(unsigned long swapPage) {
    if (swapPage >= mSwapSpacePage)
        return;
    if (mBitmap[swapPage]) {
        mSwapUsedPage--;
        mBitmap[swapPage] = false;
    }
}

bool FS::dumpPageIntoSwap(const void *src, unsigned long swapPage) {
    if (swapPage >= mSwapAllocPage)
        return false;
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    std::ofstream dump;
    dump.open(getPath(mSwapPath), std::ios::in | std::ios::out | std::ios::binary);
    if (dump.fail())
        return false;
    if (!src) {
        dump.close();
        return false;
    }
    dump.seekp(swapPage << BIT, std::ios::beg);
    dump.write((char *)src, PAGE);dump.close();
    dump.close();
    return true;
}

bool FS::loadPageIntoMemory(const void * dst, unsigned long swapPage) {
    if (swapPage >= mSwapAllocPage)
        return false;
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    std::ifstream loader;
    loader.open(getPath(mSwapPath), std::ios::in | std::ios::binary);
    if (loader.fail())
        return false;
    if (!dst) {
        loader.close();
        return false;
    }
    loader.seekg(swapPage << BIT, std::ios::beg);
    loader.read((char *)dst, PAGE);
    loader.close();
    return true;
}

unsigned long FS::getFileSize(const std::string &path) {
    if (!exists(getPath(path)))
        return 0;
    return file_size(getPath(path));
}

bool FS::readFile(const std::string &path, const void *dst, unsigned long size, unsigned long start) {
    if (!isExist(path))
        return false;
    std::ifstream reader;
    reader.open(getPath(path), std::ios::in | std::ios::binary);
    if (reader.fail())
        return false;
    if (!dst && size) {
        reader.close();
        return false;
    }
    reader.seekg((long long int)start, std::ios::beg);
    reader.read((char *)dst, size);
    reader.close();
    return true;
}

char FS::readFileByte(const std::string &path, unsigned long offset) {
    char c;
    if (!readFile(path, &c, 1, offset))
        return 0;
    return c;
}

bool FS::writeFile(const std::string &path, const void *src, unsigned long size, unsigned long start, bool trunc) {
    std::ofstream writer;
    if (!isExist(path) || trunc) {
        writer.open(getPath(path), std::ios::out | std::ios::binary);
        if (writer.is_open())
            writer.close();
        else
            return false;
    }
    writer.open(getPath(path), std::ios::in | std::ios::out | std::ios::binary);
    if (!writer.is_open()) {
        return false;
    }
    writer.seekp((long long int)start, std::ios::beg);
    if (src != nullptr)
        writer.write((char *) src, size);
    writer.close();
    return true;
}

bool FS::appendFile(const std::string &path, const void *src, unsigned long size) {
    std::ofstream appender;
    appender.open(getPath(path), std::ios::out | std::ios::app | std::ios::binary);
    if (!appender.is_open())
        return false;
    if (src)
        appender.write((char *) src, size);
    appender.close();
    return true;
}

void FS::format() {
    if (!instance)
        return;
    std::string path(instance->mBasePath.c_str());
    if (exists(path))
        remove_all(path);
    destroy();
}

unsigned long FS::getDirectorySize(const std::string &path) {
    if (!isExist(path))
        return 0;

    boost::filesystem::path rootPath(getPath(path));
    unsigned long size = 0;
    recursive_directory_iterator end_iter;
    for (recursive_directory_iterator iter(rootPath); iter != end_iter; iter++) {
        try {
            if (is_directory(*iter))
                size += getDirectorySize(iter->path().string());
            else
                size += file_size(*iter);
        } catch (const std::exception &ex) {
            std::cerr << "DIRECTORY ITERATOR ERROR: " << ex.what() << std::endl;
            continue;
        }
    }
    return size;
}

std::vector<std::string> FS::getFileNames(const std::string &path) {
    std::vector<std::string> names;
    if (isDirectory(path)) {
        directory_iterator end_iter;
        for (directory_iterator iter(getPath(path)); iter != end_iter; iter++) {
            std::string absolutePath = iter->path().string();
            unsigned long index = absolutePath.find(mBasePath.string());
            if (index != std::string::npos) {
                absolutePath = absolutePath.substr(mBasePath.string().length());
            }
            names.emplace_back(absolutePath);
        }
    } else if (isFile(path)) {
        names.emplace_back(path);
    }
    return names;
}