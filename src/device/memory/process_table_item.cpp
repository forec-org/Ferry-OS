//
// Created by 王耀 on 2017/9/18.
//

#include <iostream>
#include "process_table_item.h"
#include "config.h"
#include "mmu.h"

ProcessTableItem::ProcessTableItem(unsigned int pid, unsigned long pageCount, unsigned long stackPageCount) {
    mPid = pid;
    mPageCount = pageCount;
    mStackPageCount = stackPageCount;
    mHeapUsedSize = 0;
    mHeapAddress = 0;
    mPageTable.clear();
}

unsigned int ProcessTableItem::getPid() {
    return mPid;
}

void ProcessTableItem::insertPage(PageTableItem *pti) {
    mPageTable.insert(std::make_pair(pti->getLogicalPage(), pti));
}

bool ProcessTableItem::operator==(const ProcessTableItem & other) {
    return mPid == other.mPid;
}

void ProcessTableItem::setHeapAddress(unsigned long heapAddress) {
    mHeapAddress = heapAddress;
}

unsigned long ProcessTableItem::getHeapUsedSize() {
    return mHeapUsedSize;
}

void ProcessTableItem::updatePageTable(unsigned long logicalPage, unsigned long physicalAddress) {
    auto item = mPageTable.find(logicalPage);
    if (item == mPageTable.end() || !item->second)
        return;
    PageTableItem *pti = item->second;
    pti->setPhysicalAddress(physicalAddress);
    if (pti->isSwapped())
        pti->swapIntoMemory();
    pti->setInMemory();
    pti->setUsed();
}

unsigned long ProcessTableItem::getPhysicalAddress(unsigned long logicalAddress) {
    unsigned long physicalPage = getPhysicalPage(logicalAddress);
    if (physicalPage)
        return physicalPage + (logicalAddress & (1 << Config::getInstance()->MEM.DEFAULT_PAGE_BIT) - 1);
    return 0;
}

unsigned long ProcessTableItem::getPhysicalPage(unsigned long logicalAddress) {
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long logicalPage = logicalAddress >> BIT;
    auto pageTableItem = mPageTable.find(logicalPage);
    if (mPageTable.end() == pageTableItem)
        return 0;
    return pageTableItem->second->getPhysicalAddress();
}

unsigned long ProcessTableItem::getStackBaseAdderss() {
    return (mPageCount << Config::getInstance()->MEM.DEFAULT_PAGE_BIT) - 1;
}

bool ProcessTableItem::isValid(unsigned long logicalAddress) {
    return (logicalAddress >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT) < mPageCount;
}

bool ProcessTableItem::canAlloc(unsigned long size) {
    std::vector<unsigned long> check = canUsePages(size);
    if (check.size())
        return true;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long headPointer = (unsigned long)mHeapAddress + mHeapUsedSize;
    unsigned long stackPointer = (mPageCount - mStackPageCount) << BIT;
    return headPointer + size < stackPointer;
}

uint8_t ProcessTableItem::readByte(unsigned long logicalAddress) {
    if (!isValid(logicalAddress))
        return 0;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long logicalPage = logicalAddress >> BIT;
    auto pti = mPageTable.find(logicalPage);
    if (pti == mPageTable.end() || !pti->second)
        return 0;

    MMU::getInstance()->usePage(logicalPage, mPid);

    // 要读取的页不在内存中，需触发 MMU 调页
    if (!pti->second->isInMemory()) {
        if (!MMU::getInstance()->pageFault(mPid, logicalAddress >> BIT))
            return 0;
        return readByte(logicalAddress);
    }

    // 读取的页已在内存中，将物理地址以指针方式返回
    return *(uint8_t *)(pti->second->getPhysicalAddress() + (logicalAddress & ((1 << BIT) - 1)));
}

uint32_t ProcessTableItem::readHalfWord(unsigned long logicalAddress) {
    if (!isValid(logicalAddress))
        return 0;

    // 非对齐地址
    if (logicalAddress & 0x0003) {
        uint32_t value = 0;
        for (unsigned long ad = 0; ad <= 3; ad++) {
            value = (value << 8) | readByte(logicalAddress + 3 - ad);
        }
        return value;
    }

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long logicalPage = logicalAddress >> BIT;
    auto pti = mPageTable.find(logicalPage);
    if (pti == mPageTable.end() || !pti->second)
        return 0;

    MMU::getInstance()->usePage(logicalPage, mPid);

    // 要读取的页不在内存中，需触发 MMU 调页
    if (!pti->second->isInMemory()) {
        if (MMU::getInstance()->pageFault(mPid, logicalAddress >> BIT))
            return readHalfWord(logicalAddress);
        return 0;
    }

    // 读取的页已在内存中，将物理地址以指针方式返回
    return *(uint32_t *)(pti->second->getPhysicalAddress() + (logicalAddress & ((1 << BIT) - 1)));
}

uint64_t ProcessTableItem::readWord(unsigned long logicalAddress) {
    if (!isValid(logicalAddress))
        return 0;

    // 非对齐地址
    if (logicalAddress & 0x0007) {
        uint64_t value = 0;
        value = (value << 32) | readHalfWord(logicalAddress + 4);
        value = (value << 32) | readHalfWord(logicalAddress);
        return value;
    }

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long logicalPage = logicalAddress >> BIT;
    auto pti = mPageTable.find(logicalPage);
    if (pti == mPageTable.end() || !pti->second)
        return 0;

    MMU::getInstance()->usePage(logicalPage, mPid);

    // 要读取的页不在内存中，需触发 MMU 调页
    if (!pti->second->isInMemory()) {
        if (MMU::getInstance()->pageFault(mPid, logicalAddress >> BIT))
            return readWord(logicalAddress);
        return 0;
    }

    // 读取的页已在内存中，将物理地址以指针方式返回
    return *(uint64_t *)(pti->second->getPhysicalAddress() + (logicalAddress & ((1 << BIT) - 1)));
}


unsigned long ProcessTableItem::getAllocButNotUsedSize() {
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    auto used = (unsigned int)(mHeapUsedSize & ((1 << BIT) - 1));
    if (!used)
        return 0;
    return (1 << BIT) - used;
}

unsigned long ProcessTableItem::getHeapHeader() {
//    std::cerr << "卧槽 pid = " << mPid << ", mHeapAddress = " << mHeapAddress << ", mHeapUsedSize = " << mHeapUsedSize << std::endl;
    return mHeapAddress + mHeapUsedSize;
}

bool ProcessTableItem::write(unsigned long logicalAddress, const void *src, unsigned long size) {
    if (!isValid(logicalAddress) || !isValid(logicalAddress + size - 1)) {
        return false;
    }

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned long logicalPage = logicalAddress >> BIT;
    unsigned long firstSize = PAGE - (logicalAddress & (PAGE - 1));
    firstSize = size > firstSize ? firstSize : size;
    unsigned long leftSize = size - firstSize;
    unsigned long pages = leftSize >> BIT;
    unsigned long index = 0;
    if (leftSize & (PAGE - 1))
        pages++;
    if (firstSize)
        pages++;

    for (unsigned long pageNumber = 0; pageNumber < pages; pageNumber++) {
        auto pti = mPageTable.find(pageNumber + logicalPage);
        if (pti == mPageTable.end() || !pti->second) {
            return false;
        }

        MMU::getInstance()->usePage(pageNumber + logicalPage, mPid);

        // 要写的页不在内存中，触发调页
        if (!pti->second->isInMemory()) {
            if (!MMU::getInstance()->pageFault(mPid, pageNumber + logicalPage))
                return false;
        }

        unsigned long dst = pti->second->getPhysicalAddress();

        // 第一页，需要将该页剩余空闲空间填满
        if (pageNumber == 0) {
            // 获取开始写的物理地址
            dst += logicalAddress & (PAGE - 1);
            memcpy((void *)dst, src, firstSize);
            index += firstSize;
        } else {
            unsigned long toWriteSize = leftSize > PAGE ? PAGE : leftSize;
            memcpy((void *) dst, (void *)((unsigned long)src + index), toWriteSize);
            index += toWriteSize;
            leftSize -= toWriteSize;
        }
    }
    return true;
}

// 从堆区域的已释放内存中获取可用页并间断分配
std::vector<unsigned long> ProcessTableItem::canUsePages(unsigned long size) {
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // 计算该进程堆首地址和末尾地址所在的页
    unsigned long heapPage = mHeapAddress >> BIT;
    if (mHeapAddress & ((1 << BIT) - 1))
        heapPage++;
    unsigned long endPage = (mHeapAddress + mHeapUsedSize) >> BIT;
    if ((mHeapAddress + mHeapUsedSize) & ((1 << BIT) - 1))
        endPage++;

    // 计算需要分配的页大小
    unsigned long page = size >> BIT;
    if (size & ((1 << BIT) - 1))
        page++;

    unsigned long index = heapPage;
    unsigned int totalCount = 0;
    std::vector<unsigned long> pages;
    pages.clear();

    // 检查堆区域的每一页是否被占用，不被占用则加入可复用页
    while (index < endPage) {
        auto item = mPageTable.find(index);
        if (item == mPageTable.end())
            return std::vector<unsigned long>();
        if (!item->second->isUsed()) {
            unsigned int count = 0;
            do {
                // 该页可用，将其推入可用向量
                pages.emplace_back(index);
                count++;
                auto _item = mPageTable.find(index + count);
                if (_item == mPageTable.end())
                    return std::vector<unsigned long>();
            } while (index + count < endPage);
            totalCount += count;

            // 可用页数已超过所需页数
            if (totalCount >= page) {
                return pages;
            }
            index += count;
        }
        index++;
    }
    return std::vector<unsigned long>();
}

PageTableItem* ProcessTableItem::getPageTableItem(unsigned long logicalPage) {
    auto item = mPageTable.find(logicalPage);
    if (item == mPageTable.end())
        return nullptr;
    return item->second;
}

void ProcessTableItem::setProcessTableNumber(unsigned int processTableNumber) {
    mProcessTableNumber = processTableNumber;
}

unsigned int ProcessTableItem::getProcessTableNumber() {
    return mProcessTableNumber;
}

unsigned long ProcessTableItem::getPageCount() {
    return mPageCount;
}

void ProcessTableItem::setHeapUsedSize(unsigned long heapUsedSize) {
    mHeapUsedSize = heapUsedSize;
}