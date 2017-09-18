//
// Created by 王耀 on 2017/9/18.
//

#include "mmu.h"
#include <iostream>

MMU *MMU::instance = nullptr;
char *MMU::mBootAddress = nullptr;
long MMU::mSwapBaseAddress = 0;
unsigned long MMU::mCapacity = _MEM_DEFAULT_CAPACITY;

MMU *MMU::getInstance() {
    if (instance)
        return instance;
    MMU::init(mCapacity);
    return instance;
}

MMU::MMU() {
    mFrameTable.clear();
    mProcessTable.clear();
    mPageTable = nullptr;
    for (unsigned long i = 0; i < MMU::mCapacity; i++)
        mFrameTable.insert(mFrameTable.end(), FrameTableItem(i << Config::getInstance()->MEM.DEFAULT_PAGE_BIT));
    mSystemUsedSize = 0;
}

MMU::~MMU() {
    delete mPageTable;
    if (mSystemAddress)
        for (unsigned int i = 0; i < mSystemUsedSize; i++)
            delete []mSystemAddress[i];
    delete []mSystemAddress;
    mFrameTable.clear();
    mProcessTable.clear();
}

void MMU::init(unsigned long capacity) {
    delete instance;
    mCapacity = capacity;
    mBootAddress = new char[Config::getInstance()->OS.BOOT_MEMORY_KB << 10];
    instance = new(mBootAddress) MMU();
    auto pointer = long(instance);
    if (pointer & ((1 << (Config::getInstance()->MEM.DEFAULT_PAGE_BIT + 1)) - 1)) {
        pointer >>= Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
        pointer <<= (1 + Config::getInstance()->MEM.DEFAULT_PAGE_BIT);
    }

    unsigned long pageTableSize = Config::getInstance()->MEM.DEFAULT_CAPACITY * sizeof(PageTableItem);
    auto pageTablePage = (unsigned int)(pageTableSize >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT) + 1;

    instance->mSystemAddress = new((char*)pointer) char[(Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE - pageTablePage) << Config::getInstance()->MEM.DEFAULT_PAGE_BIT];
    memset(instance->mSystemAddress, 0, sizeof((Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE - pageTablePage) << Config::getInstance()->MEM.DEFAULT_PAGE_BIT));

    auto *baseAddress = new char[pageTablePage << Config::getInstance()->MEM.DEFAULT_PAGE_BIT];
    instance->mPageTable = new(baseAddress) PageTable(mCapacity, baseAddress + sizeof(PageTable));
    instance->mSystemUsedSize = pageTablePage;
}

void MMU::destroy() {
    delete instance;
    delete []mBootAddress;
}

bool MMU::check() {
    bool valid = true;
    std::cerr << sizeof(MMU) << std::endl;
    if ((Config::getInstance()->OS.BOOT_MEMORY_KB << 10) < sizeof(MMU)) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Error: OS.BOOT_MEMORY_KB is too small for booting!" << std::endl;
        valid = false;
    }
    if (Config::getInstance()->MEM.DEFAULT_PAGE_BIT > 24) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Error: MEM.DEFAULT_PAGE_BIT should smaller than 24 or a page will exceed 16MB." << std::endl;
        valid = false;
    }
    unsigned long pageTableSize = Config::getInstance()->MEM.DEFAULT_CAPACITY * sizeof(PageTableItem) + sizeof(PageTable);
    unsigned int pageTablePage = (unsigned int)(pageTableSize >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT) + 1;
    if (pageTablePage > Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Error: OS.MEM.DEFAULT_OS_USED_PAGE is too small for a page table." << std::endl;
        valid = false;
    }
    return valid;
}

void* MMU::allocSystemMemory(unsigned long size) {
    auto pageCount = (unsigned int)(size >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT);
    if (size & ((1 << Config::getInstance()->MEM.DEFAULT_PAGE_BIT + 1) - 1))
        pageCount++;
    if (mSystemUsedSize + pageCount > Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE) {
        return nullptr;
    }
    for (unsigned int i = 0; i < pageCount; i++)
        mPageTable->insertPage(i << Config::getInstance()->MEM.DEFAULT_PAGE_BIT, 0, (long)&mSystemAddress[mSystemUsedSize + (i << Config::getInstance()->MEM.DEFAULT_PAGE_BIT)]);
    mSystemUsedSize += pageCount;
    return (void *)&mSystemAddress[mSystemUsedSize - pageCount];
}

long MMU::allocUserMemory(unsigned int pid, unsigned long size) {
    // TODO
    return 0;
}