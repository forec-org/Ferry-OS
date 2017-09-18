//
// Created by 王耀 on 2017/9/18.
//

#include "page_table_item.h"

PageTableItem::PageTableItem(long logicalAddress, unsigned int pid, long physicalAddress) {
    mChanged = false;
    mInMemory = false;
    mSwapped = false;
    mSystem = false;
    mLogicalAddress = logicalAddress;
    mPhysicalAddress = physicalAddress;
    mPid = pid;
    pre = next = nullptr;
}

bool PageTableItem::operator==(const PageTableItem & other) {
    return mLogicalAddress == other.mLogicalAddress;
}

long PageTableItem::getPhysicalAddress() {
    return mPhysicalAddress;
}

long PageTableItem::getLogicalAddress() {
    return mLogicalAddress;
}

unsigned int PageTableItem::getPid() {
    return mPid;
}

void PageTableItem::setLogicalAddress(long logicalAddress) {
    mLogicalAddress = logicalAddress;
}

void PageTableItem::setPhysicalAddress(long physicalAddress) {
    mPhysicalAddress = physicalAddress;
}

void PageTableItem::setPid(unsigned int pid) {
    mPid = pid;
}

std::pair<long, unsigned int> PageTableItem::getID() {
    return std::make_pair(mLogicalAddress, mPid);
}

void PageTableItem::reset() {
    mSwapAddress = 0;
    mSwapped = false;
    mChanged = false;
    mInMemory = false;
    mSystem = false;
}

char* PageTableItem::read() {
    // TODO
    return nullptr;
}

void PageTableItem::write(char *src, unsigned int size) {
    // TODO
}

bool PageTableItem::swapIntoMemory() {
    // TODO
    return true;
}

bool PageTableItem::swapOutMemory() {
    // TODO
    return true;
}
