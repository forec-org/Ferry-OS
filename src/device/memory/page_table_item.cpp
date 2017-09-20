//
// Created by 王耀 on 2017/9/18.
//

#include "mmu.h"
#include "page_table_item.h"

PageTableItem::PageTableItem(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress) {
    mChanged = false;
    mInMemory = false;
    mSwapped = false;
    mSystem = false;
    mHosted = false;
    mUsed = false;
    mLogicalPage = logicalPage;
    mPhysicalAddress = physicalAddress;
    mSwapAddress = 0;
    mPid = pid;
    pre = next = nullptr;
}

bool PageTableItem::operator==(const PageTableItem & other) {
    return mLogicalPage == other.mLogicalPage && mPid == other.mPid;
}

unsigned long PageTableItem::getPhysicalAddress() {
    return mPhysicalAddress;
}

unsigned long PageTableItem::getLogicalPage() {
    return mLogicalPage;
}

unsigned int PageTableItem::getPid() {
    return mPid;
}

void PageTableItem::setLogicalPage(unsigned long logicalPage) {
    mLogicalPage = logicalPage;
}

void PageTableItem::setPhysicalAddress(unsigned long physicalAddress) {
    mPhysicalAddress = physicalAddress;
}

void PageTableItem::setPid(unsigned int pid) {
    mPid = pid;
}

std::pair<long, unsigned int> PageTableItem::getID() {
    return std::make_pair(mLogicalPage, mPid);
}

void PageTableItem::reset() {
    mSwapAddress = 0;
    mSwapped = false;
    mChanged = false;
    mInMemory = false;
    mSystem = false;
    mUsed = false;
}

bool PageTableItem::swapIntoMemory() {
    if (mSwapAddress == 0)
        return true;
// TODO
    return false;
}

bool PageTableItem::swapOutMemory() {
    if (mSwapAddress == 0)
        mSwapAddress = MMU::getInstance()->getAvailableSwapAddress();

    // 交换空间已满，无法换页
    if (!mSwapAddress)
        return false;

    // 换出到 Swap 空间
    if (mPhysicalAddress != 0) {
        // TODO
    }
    mSwapped = true;

    // 将原始空间填充 0
    if (mPhysicalAddress != 0) {
        unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
        auto *dst = (void *)mPhysicalAddress;
        memset(dst, 0, PAGE);
    }
    return true;
}
