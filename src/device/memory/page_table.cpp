//
// Created by 王耀 on 2017/9/18.
//

#include "page_table.h"

PageTable::PageTable(unsigned long capacity, char *baseAddress) {
    mPageTableMap.clear();
    mPageTableBaseAddress = baseAddress;
    mLRUStackHead = mLRUStackTail = nullptr;
    mCapacity = capacity;
    mUsed = 0;
}

PageTableItem* PageTable::getPageTableItem(long logicalAddress, unsigned int pid) {
    auto pti = mPageTableMap.find(std::make_pair(logicalAddress, pid));
    return pti == mPageTableMap.end() ? nullptr : pti->second;
}

void PageTable::insertPage(long logicalAddress, unsigned int pid, long physicalAddress) {
    if (mUsed >= mCapacity)
        schedule(logicalAddress, pid, physicalAddress);
    auto pointer = mPageTableBaseAddress + sizeof(PageTableItem) * mUsed;
    auto *pti = new(pointer) PageTableItem(logicalAddress, pid, physicalAddress);
    mPageTableMap[std::make_pair(logicalAddress, pid)] = pti;
    pti->next = mLRUStackHead;
    if (mLRUStackHead)
        mLRUStackHead->pre = pti;
    if (!mLRUStackTail)
        mLRUStackTail = pti;
    mLRUStackHead = pti;
}

void PageTable::schedule(long logicalAddress, unsigned int pid, long physicalAddress) {
    auto pointer = mLRUStackTail;
    while (pointer && pointer->isSystem()) {
        pointer = pointer->pre;
        if (pointer == mLRUStackTail) {
            mLRUStackTail->swapOutMemory();
            break;
        }
    }
    if (!pointer)
        return;
    mPageTableMap.erase(pointer->getID());
    pointer->setLogicalAddress(logicalAddress);
    pointer->setPhysicalAddress(physicalAddress);
    pointer->setPid(pid);
    pointer->reset();
    if (pid == 0)
        pointer->setSystem();
    if (pointer->pre)
        pointer->pre->next = pointer->next;
    if (pointer->next)
        pointer->next->pre = pointer->pre;
    pointer->next = mLRUStackHead;
    if (mLRUStackHead)
        mLRUStackHead->pre = pointer;
    mLRUStackHead = pointer;
}