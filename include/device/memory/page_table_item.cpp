//
// Created by 王耀 on 2017/9/18.
//

#include "page_table_item.h"
#include "mmu.h"
#include "../../device/fs/fs.h"

PageTableItem::PageTableItem(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress) {
    mChanged = false;
    mInMemory = false;
    mSwapped = false;
    mHaveSwapped = false;
    mSystem = false;
    mHosted = false;
    mUsed = false;
    mLogicalPage = logicalPage;
    mPhysicalAddress = physicalAddress;
    mSwapPage = FS::getInstance()->getSwapSpacePage();
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
    mChanged = false;
    mInMemory = false;
    mSystem = false;
    mUsed = false;
}

unsigned long PageTableItem::getSwapPage() {
    return mSwapPage;
}

bool PageTableItem::swapIntoMemory() {
    if (!mPhysicalAddress)
        return false;
    if (mSwapPage == FS::getInstance()->getSwapSpacePage())
        return false;
    FS::getInstance()->loadPageIntoMemory((void*)mPhysicalAddress, mSwapPage);
    setInMemory();
    mSwapped = false;
    return true;
}

bool PageTableItem::swapOutMemory() {
    if (mSwapPage == FS::getInstance()->getSwapSpacePage())
        mSwapPage = FS::getInstance()->allocSwapPage();

    // 交换空间已满，无法换页
    if (mSwapPage == FS::getInstance()->getSwapSpacePage())
        return false;

    // 换出到 Swap 空间
    if (mPhysicalAddress) {
        FS::getInstance()->dumpPageIntoSwap((void*)mPhysicalAddress, mSwapPage);
        setSwapped();
        mHaveSwapped = true;
    }

    mInMemory = false;
    // 将原始空间填充 0
    if (mPhysicalAddress != 0) {
        unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
        auto *dst = (void *)mPhysicalAddress;
        memset(dst, 0, PAGE);
    }
    return true;
}
