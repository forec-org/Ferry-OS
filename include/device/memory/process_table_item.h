//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PROCESS_TABLE_ITEM_H
#define SDOS_PROCESS_TABLE_ITEM_H

#include "../../config/macro.h"
#include "page_table_item.h"
#include <unordered_map>

class ProcessTableItem {
private:
    unsigned long mPageCount;
    unsigned long mStackPageCount;
    long          mBaseAddress;
    long          mHeapAddress;
    unsigned int  mPid;
    std::unordered_map<long, PageTableItem*> mPageTable;
public:
    explicit ProcessTableItem(unsigned int pid,
                              unsigned long pageCount = _MEM_DEFAULT_PROCESS_PAGE,
                              unsigned long stackPageCount = _MEM_DEFAULT_STACK_PAGE);
    void setBaseAddress(long baseAddress);
    void *getStackBasePointer();
    void *read(long logicalAddress);
    bool write(long logicalAddress, const void *src, unsigned long size);
    long getPhysicalAddress(long logicalAddress);
    bool operator == (const ProcessTableItem&);
};

#endif //SDOS_PROCESS_TABLE_ITEM_H
