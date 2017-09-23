//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PROCESS_TABLE_ITEM_H
#define SDOS_PROCESS_TABLE_ITEM_H

#include "../../config/macro.h"
#include "page_table_item.h"
#include <vector>
#include <unordered_map>
#include <cstdint>

class ProcessTableItem {
private:
    unsigned long mPageCount;
    unsigned long mStackPageCount;
    unsigned long mHeapAddress;
    unsigned long mHeapUsedSize;
    unsigned int  mProcessTableNumber;
    unsigned int  mPid;
    // 进程创建时将所有虚拟页添加至进程独有的虚拟页表中
    std::unordered_map<long, PageTableItem*> mPageTable;
public:
    explicit ProcessTableItem(unsigned int pid,
                              unsigned long pageCount = _MEM_DEFAULT_PROCESS_PAGE,
                              unsigned long stackPageCount = _MEM_DEFAULT_STACK_PAGE);
    void setProcessTableNumber(unsigned int processTableNumber);
    unsigned int getProcessTableNumber();
    unsigned int getPid();
    void setHeapAddress(unsigned long heapAddress);
    void setHeapUsedSize(unsigned long heapUsedSize);
    void insertPage(PageTableItem *pti);
    void updatePageTable(unsigned long logicalPage, unsigned long physicalAddress);
    unsigned long getPhysicalAddress(unsigned long logicalAddress);
    unsigned long getPhysicalPage(unsigned long logicalAddress);
    unsigned long getPageCount();
    unsigned long getStackBaseAdderss();
    PageTableItem * getPageTableItem(unsigned long logicalPage);
    unsigned long getAllocButNotUsedSize();
    unsigned long getHeapHeader();
    unsigned long getHeapUsedSize();
    std::vector<unsigned long> canUsePages(unsigned long size);
    bool isValid(unsigned long logicalAddress);
    bool canAlloc(unsigned long size);
    uint8_t readByte(unsigned long logicalAddress);
    uint32_t readHalfWord(unsigned long logicalAddress);
    uint64_t readWord(unsigned long logicalAddress);
    bool write(unsigned long logicalAddress, const void *src, unsigned long size);
    bool operator == (const ProcessTableItem&);
};

#endif //SDOS_PROCESS_TABLE_ITEM_H
