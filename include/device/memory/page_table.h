//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PAGE_TABLE_H
#define SDOS_PAGE_TABLE_H

#include "page_table_item.h"
#include <unordered_map>
#include <utility>

class PageTable {
private:
    std::unordered_map<std::pair<long ,unsigned int>, PageTableItem *> mPageTableMap;
    PageTableItem *mLRUStackHead, *mLRUStackTail;
    char * mPageTableBaseAddress;
    unsigned long mCapacity;
    unsigned int mUsed;
public:
    explicit PageTable(unsigned long capacity, char *baseAddress);
    void insertPage(long logicalAddress, unsigned int pid, long physicalAddress);
    void schedule(long logicalAddress, unsigned int pid, long physicalAddress);
    PageTableItem *getPageTableItem(long logicalAddress, unsigned int pid);
};

#endif //SDOS_PAGE_TABLE_H
