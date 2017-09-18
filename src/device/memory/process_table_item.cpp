//
// Created by 王耀 on 2017/9/18.
//

#include "process_table_item.h"

ProcessTableItem::ProcessTableItem(unsigned int pid, unsigned long pageCount, unsigned long stackPageCount) {
    mPid = pid;
    mPageCount = pageCount;
    mStackPageCount = stackPageCount;
    mPageTable.clear();
}

bool ProcessTableItem::operator==(const ProcessTableItem & other) {
    return mPid == other.mPid;
}

void ProcessTableItem::setBaseAddress(long baseAddress) {
    mBaseAddress = baseAddress;
}

long ProcessTableItem::getPhysicalAddress(long logicalAddress) {
    auto pageTableItem = mPageTable.find(logicalAddress>>4);
    if (pageTableItem == mPageTable.end())
        return 0;
    return pageTableItem->second->getPhysicalAddress();
}

void *ProcessTableItem::getStackBasePointer() {
    return getPhysicalAddress();
}

void *ProcessTableItem::read(long logicalAddress) {

}