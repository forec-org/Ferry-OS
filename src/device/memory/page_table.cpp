//
// Created by 王耀 on 2017/9/18.
//

#include <iostream>
#include "config.h"
#include "page_table.h"
#include "mmu.h"

PageTable::PageTable(unsigned long capacity, char *baseAddress) {
    mPageTableMap.clear();
    mPageTableBaseAddress = baseAddress;
    mLRUStackHead = mLRUStackTail = nullptr;
    mCapacity = capacity;
    mAlloced = mUsed = 0;
}

PageTableItem* PageTable::getPageTableItem(unsigned long logicalPage, unsigned int pid) {
    auto pti = mPageTableMap.find(std::make_pair(logicalPage, pid));
    return pti == mPageTableMap.end() ? nullptr : pti->second;
}

bool PageTable::isFull() {
    return mUsed >= mCapacity;
}

void PageTable::deletePageTableItem(unsigned long logicalPage, unsigned int pid) {
    PageTableItem * pti = getPageTableItem(logicalPage, pid);
    if (pti) {
        pti->clearHosted();
        pti->clearUsed();
        mUsed--;
    }
}

void PageTable::clearPageForPid(unsigned int pid) {
    for (auto item: mPageTableMap) {
        if (item.second->getPid() == pid) {
            if (item.second->isUsed()) {
                item.second->clearHosted();
                item.second->clearUsed();
                mUsed--;
            }
        }
    }
}

// 此方法在进程尚未分配内存时使用，首先检查有无已分配的可用帧，否则创建新区域或置换
bool PageTable::insertPage(unsigned long logicalPage, unsigned int pid) {
    if (mUsed < mCapacity) {
        // 有无已经分配内存且未被占用的帧
        FrameTableItem * fti = MMU::getInstance()->getAvailableFrame();
        if (fti)
            return insertPage(logicalPage, pid, fti->getFrameAddress());
        unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
        // 有无未被占用且尚未分配内存的帧
        fti = MMU::getInstance()->getNeedAllocFrame();
        if (fti) {
            auto * space = new char[PAGE];
            fti->setFrameAddress((unsigned long)space);
            return insertPage(logicalPage, pid, fti->getFrameAddress());
        }

        // 因为帧表已经在 BOOT 区域完整创建，如果上述两部找不到说明帧表已满，这与页表不满冲突
        return false;
    }
    return schedule(logicalPage, pid);
}

bool PageTable::insertPage(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress) {
    if (mUsed >= mCapacity)
        return schedule(logicalPage, pid);

    // 计算分配 PageTableItem 的位置，使用系统独占内存
    auto pointer = mPageTableBaseAddress + sizeof(PageTableItem) * mUsed;

    if (mUsed < mAlloced) {
        // 先检查有没有不被占用的页
        for (auto item: mPageTableMap) {
            if (item.second->isUsed()) {
                pointer = (char *) item.second;
                mPageTableMap.erase(item.first);
                break;
            }
        }
    } else {
        mAlloced++;
    }
    auto *pti = new(pointer) PageTableItem(logicalPage, pid, physicalAddress);

    // 将新创建的页表条目插入页表
    mPageTableMap[std::make_pair(logicalPage, pid)] = pti;

    // 更新 LRU 信息，将该条目放置在 LRU 队列的最头部
    pti->next = mLRUStackHead;
    if (mLRUStackHead)
        mLRUStackHead->pre = pti;
    if (!mLRUStackTail)
        mLRUStackTail = pti;
    mLRUStackHead = pti;

    // 通知 MMU 更新帧表和进程内页表
    MMU::getInstance()->updateAll(logicalPage, pid, physicalAddress);

    // 更新页表已被占用的数量，并设置新的条目已被使用
    mUsed++;
    pti->setUsed();
    if (!pid) {
        pti->setSystem();
        pti->setHosted();
    }
    return true;
}

bool PageTable::schedule(unsigned long logicalPage, unsigned int pid) {
    auto pointer = mLRUStackTail;
    while (pointer && pointer->isHosted()) {
        pointer = pointer->pre;
        if (pointer == mLRUStackTail) {
            // 找不到任何可置换的页，这种情况仅在操作系统内存与整个模拟内存相等时出现
            return false;
        }
    }

    // 页表中存在空指针
    if (!pointer)
        return false;

    // 将要被置换的页从页表中移除并更新该页对应的进程表信息
    mPageTableMap.erase(pointer->getID());
    unsigned int opid = pointer->getPid();
    unsigned long ologicalPage = pointer->getLogicalPage();
    PageTableItem *opti = MMU::getInstance()->getProcessPageTableItem(opid, ologicalPage);
    if (opti)
        opti->swapOutMemory();

    // 更新该页表项信息，将数据指向新进程的逻辑地址
    pointer->setLogicalPage(logicalPage);
    pointer->setPid(pid);
    pointer->reset();
    pointer->setUsed();
    if (pid == 0) {
        pointer->setSystem();
        pointer->setHosted();
    }

    // 更新 LRU 链表中该页表项的信息，将该页插入头部
    if (pointer->pre)
        pointer->pre->next = pointer->next;
    if (pointer->next)
        pointer->next->pre = pointer->pre;
    pointer->next = mLRUStackHead;
    if (mLRUStackHead)
        mLRUStackHead->pre = pointer;
    mLRUStackHead = pointer;

    // 将更新后的页插入页表
    mPageTableMap.insert(std::make_pair(std::make_pair(logicalPage, pid), pointer));

    // 通知 MMU 更新各项信息（包括换入内存等操作）
    MMU::getInstance()->updateAll(logicalPage, pid, pointer->getPhysicalAddress());
    return true;
}

void PageTable::stdErrPrint() {
    for (auto item: mPageTableMap) {
        auto id = item.first;
        auto pti = item.second;
        std::cerr << "logical = " << id.first << ", pid = " << id.second << ", physical = " << pti->getPhysicalAddress() << std::endl;
    }
}