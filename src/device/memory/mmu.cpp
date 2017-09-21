//
// Created by 王耀 on 2017/9/18.
//

#include "mmu.h"
#include <iostream>

MMU *MMU::instance = nullptr;
char *MMU::mBootAddress = nullptr;
unsigned long MMU::mSwapBaseAddress = 0;
unsigned long MMU::mCapacity = _MEM_DEFAULT_CAPACITY;

MMU *MMU::getInstance() {
    if (instance)
        return instance;
    MMU::init(mCapacity);
    return instance;
}

MMU::MMU() {
    mFrameTable.clear();
    mProcessTable.clear();
    mProcessTableUsed.clear();
    mPageTable = nullptr;
    mSystemUsedSize = 0;
}

MMU::~MMU() {
    // 无需释放系统内存区域全局页表（在最后统一释放系统内存区域）
    // delete mPageTable;

    // 根据帧表内容释放内存
    for (FrameTableItem *p : mFrameTable) {
        if (p && p->getFrameAddress() && p->getPid())
            delete [](char *)p->getFrameAddress();
        // 无需释放 Frame，在最后 destroy 时释放
        // delete p;
    }
    mFrameTable.clear();

    // 清空进程表（无需释放内存，最后统一释放系统内存区域）
    // for (auto item : mProcessTable)
    //    delete item.second;
    mProcessTable.clear();

    // 删除最早分配的系统内存
//    delete []mSystemAddress;
}

bool MMU::check() {
    bool valid = true;
    unsigned long frameTableSize = Config::getInstance()->MEM.DEFAULT_CAPACITY * sizeof(FrameTableItem);
    // BOOT 内存不足以容纳 MMU 和帧表
    if ((Config::getInstance()->OS.BOOT_MEMORY_KB << 10) < sizeof(MMU) + frameTableSize) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Error: OS.BOOT_MEMORY_KB is too small for booting!" << std::endl;
        valid = false;
    }
    // BOOT 内存过大产生浪费，非 Fatal Error
    if ((Config::getInstance()->OS.BOOT_MEMORY_KB << 10) > (sizeof(MMU) + frameTableSize) << 6) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Warning: OS.BOOT_MEMORY_KB is too big for booting!" << std::endl;
    }
    // 每页大小超过 16MB
    if (Config::getInstance()->MEM.DEFAULT_PAGE_BIT > 24) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Warning: MEM.DEFAULT_PAGE_BIT should smaller than 24 or a page will exceed 16MB." << std::endl;
    }

    // 每页大小小于 1MB，可能会使页表占空间过大，导致系统分配内存不足
    if (Config::getInstance()->MEM.DEFAULT_PAGE_BIT < 20) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Warning: MEM.DEFAULT_PAGE_BIT smaller than 20 may not pass the OS mem check." << std::endl;
    }

    // 系统内存大小不足以容纳页表、进程表
    unsigned long pageTableSize = Config::getInstance()->MEM.DEFAULT_CAPACITY * sizeof(PageTableItem) + sizeof(PageTable);
    unsigned long pageTablePage = (pageTableSize >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT) + 1;
    unsigned long processTableSize = Config::getInstance()->OS.MAXIMUM_TASK_PAGE * sizeof(PageTableItem) + sizeof(ProcessTableItem);
    processTableSize *= Config::getInstance()->OS.MAXIMUM_TASKS;
    unsigned long processTablePage = (processTableSize >> Config::getInstance()->MEM.DEFAULT_PAGE_BIT) + 1;
    if (pageTablePage + processTablePage > Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE) {
        if (Config::mShowErrorLog)
            std::cerr << "Config Error: OS.MEM.DEFAULT_OS_USED_PAGE is too small for a page&process table." << std::endl;
        valid = false;
    }
    return valid;
}

void MMU::init(unsigned long capacity) {
    destroy();
    mCapacity = capacity;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // 分配 BOOT 内存并在 BOOT 区内创建 MMU
    mBootAddress = new char[Config::getInstance()->OS.BOOT_MEMORY_KB << 10];
    instance = new(mBootAddress) MMU();

    auto pointer = (unsigned long)mBootAddress + sizeof(MMU);

    // 在 BOOT 区域分配完整帧表
    for (unsigned int i = 0; i < Config::getInstance()->MEM.DEFAULT_CAPACITY; i++) {
        auto fti = new((char *)pointer) FrameTableItem(0);
        pointer += sizeof(FrameTableItem);
        instance->mFrameTable.emplace_back(fti);
    }

    // 分配系统独占区域内存
    instance->mSystemAddress = new char[Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE << BIT];
    memset(instance->mSystemAddress, 0, sizeof(Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE << BIT));

    // 计算页表及页表项所需内存大小和页数
    unsigned long pageTableSize = Config::getInstance()->MEM.DEFAULT_CAPACITY * sizeof(PageTableItem);
    unsigned long pageTablePage = pageTableSize >> BIT;
    if (pageTableSize & ((1 << BIT) - 1))
        pageTablePage++;

    // 分配页表
    instance->mPageTable = new(instance->mSystemAddress) PageTable(mCapacity, instance->mSystemAddress + sizeof(PageTable));

    // 将页表已使用内存页计入系统已使用内存
    instance->mSystemUsedSize += pageTablePage << BIT;

    // 计算进程表及进程表项所需的内存大小和页数
    unsigned long processTableSize = Config::getInstance()->OS.MAXIMUM_TASK_PAGE * sizeof(PageTableItem) + sizeof(ProcessTableItem);
    processTableSize *= Config::getInstance()->OS.MAXIMUM_TASKS;
    unsigned long processTablePage = processTableSize >> BIT;
    if (processTableSize & ((1 << BIT) - 1))
        processTablePage++;

    // 预分配进程表
    instance->processTableAvailable = instance->mSystemAddress + instance->mSystemUsedSize;
    instance->mSystemUsedSize += processTablePage << BIT;

    // 设置固定不可释放区域
    instance->mHostedSize = instance->mSystemUsedSize;

    // 将页表占用的内存页放置入帧表和页表中
    for (unsigned long i = 0; i < pageTablePage; i++) {
        // 页表，将已分配物理内存地址置入页表
        instance->mPageTable->insertPage(i, 0, (unsigned long)instance->mSystemAddress + (i << BIT));
        // 帧表，从地址 0 开始更新内存信息
        instance->mFrameTable[i]->updateFrame((unsigned long)instance->mSystemAddress + (i << BIT), i, 0);
    }

    // 将进程表预占用的内存置入帧表和页表
    for (unsigned long i = pageTablePage; i < pageTablePage + processTablePage; i++) {
        instance->mPageTable->insertPage(i, 0, (unsigned long)instance->mSystemAddress + (i << BIT));
        instance->mFrameTable[i]->updateFrame((unsigned long)instance->mSystemAddress + (i << BIT), i, 0);
    }

    // 初始化时未分配进程，进程均未使用内存
    instance->mProcessTableUsed.resize(Config::getInstance()->OS.MAXIMUM_TASKS, false);
}

void MMU::destroy() {
//    delete instance;
    // 释放 BOOT 区域内存，系统关闭
//    delete []mBootAddress;
}

unsigned long MMU::allocSystemMemory(unsigned long size) {
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // 剩余内存不足以支持待分配内存大小
    if (mSystemUsedSize + size > Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE << BIT)
        return 0;

    // 需要新分配的页数
    unsigned long pageCount = (mSystemUsedSize + size) >> BIT;
    if ((mSystemUsedSize + size) & ((1 << BIT) - 1))
        pageCount++;
    pageCount -= (mSystemUsedSize >> BIT);

    // 需要新分配页
    if (pageCount) {
        // 获取需要新分配的页的起始地址和页号（内存已在初始化时分配完成，现在由 MMU 交付虚拟操作系统）
        auto page = (unsigned int)(mSystemUsedSize >> BIT);
        unsigned long pageAddress = page << BIT;
        if (mSystemUsedSize & ((1 << BIT) - 1)) {
            page++;
            pageAddress += Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
        }
        // 更新页表和帧表
        for (unsigned int i = 0; i < pageCount; i++) {
            mPageTable->insertPage(page + i, 0, (unsigned long) &mSystemAddress[pageAddress + (i << BIT)]);
            mFrameTable[page + i]->updateFrame((unsigned long) &mSystemAddress[pageAddress + (i << BIT)], page + i, 0);
        }
    }

    mSystemUsedSize += size;

    // 返回分配内存的起始地址（逻辑地址）
    return mSystemUsedSize - size;
}

void MMU::freeSystemMemory(unsigned long logicalAddress, unsigned long size) {
    auto ptr = (char *)getPhysicalPointer(logicalAddress);

    // 试图释放不可释放区域
    if (ptr < mSystemAddress + mHostedSize)
        return;

    // 要释放的区域开始地址超出了已分配区域
    if (ptr >= mSystemAddress + mSystemUsedSize)
        return;

    // 要释放的内存长度大于从 ptr 开始已经分配的内存大小
    if (mSystemUsedSize + mSystemAddress - ptr < size)
        size = mSystemUsedSize + mSystemAddress - ptr;

    // 将释放段之后的内存段前移
    memcpy(ptr, ptr+size, mSystemUsedSize - (ptr + size - mSystemAddress));

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // 计算释放前／后系统区域已占用内存大小和对应页号
    long newUsedSize = mSystemUsedSize - size;
    auto pageToFree = (unsigned int)(newUsedSize >> BIT);
    if (newUsedSize & ((1 << BIT) - 1))
        pageToFree++;
    auto lastPage = (unsigned int)(mSystemUsedSize >> BIT);
    if (lastPage > mFrameTable.size())
        lastPage = (unsigned int)mFrameTable.size();

    // 更新帧表，将 mSystemUsedSize - size 到 mSystemUsedSize 部分设置为未占用
    for (unsigned int i = pageToFree; i < lastPage; i++)
        mFrameTable[i]->clear();

    // 更新页表，将该部分内存在页表中的 mHosted 设为不使用、可置换
    for (unsigned int i = pageToFree; i < lastPage; i++) {
        PageTableItem *pti = mPageTable->getPageTableItem(i, 0);
        pti->clearHosted();
        pti->clearUsed();
    }

    mSystemUsedSize -= size;
}

unsigned long MMU::allocUserMemory(unsigned int pid, unsigned long size, unsigned long start) {
    auto item = mProcessTable.find(pid);
    // 未能找到对应进程
    if (item == mProcessTable.end())
        return 0;
    ProcessTableItem *prti = item->second;
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // start 位置不为 0，不在当前堆上继续分配，从指定位置开始分配内存
    if (start != 0) {
        // 从 start 处开始的剩余空间不足
        if ((prti->getPageCount() << BIT) - start < size)
            return 0;
    } else if (!prti || !prti->canAlloc(size))  // 在堆上分配且空间不足
        return 0;

    std::vector<unsigned long> pages = std::vector<unsigned long>();
    std::vector<unsigned long> interPages = std::vector<unsigned long>();
    if (start == 0)  // 获取该进程堆上空闲页
        interPages = prti->canUsePages(size);
    else {
        unsigned long startPage = (start >> BIT) + ((start & ((1 << BIT) - 1)) != 0);
        unsigned long endAddress = size + start;
        unsigned long endPage = (endAddress >> BIT) + ((endAddress & ((1 << BIT) - 1)) != 0);
        interPages = std::vector<unsigned long>();
        for (unsigned long i = startPage; i < endPage; i++)
            interPages.emplace_back(i);
    }

    // 如果从堆尾分配，分配后的返回地址，prti 将被修改，暂存返回值
    unsigned long virtualPointer = start == 0 ? prti->getHeapHeader() : start;

    // 计算需要分配的内存大小和对应的页数
    unsigned long allocButNotUsedSize = prti->getAllocButNotUsedSize();
    unsigned long needToAllocSize = size > allocButNotUsedSize ? size - allocButNotUsedSize : 0;
    unsigned long page = needToAllocSize >> BIT;
    if (needToAllocSize & ((1 << BIT) - 1))
        page++;

    // 可从中间释放内存处分配新内存
    if (!interPages.empty()) {
        pages = interPages;
        virtualPointer = start == 0 ? pages[0] << BIT : start;
    } else {
        // 计算要分配的页对应的起始页号
        unsigned long pageNumber = virtualPointer >> BIT;
        if (virtualPointer & ((1 << BIT) - 1))
            pageNumber++;

        // 将要分配的页填入 pages 表
        pages = std::vector<unsigned long>();
        for (unsigned long p = 0; p < page; p++) {
            pages.emplace_back(p + pageNumber);
        }
    }

    // 获取已被分配内存的页数，此部分可被重复利用
    unsigned int availableFrames = getAvailableFrameCount();
    unsigned long reuseFrames = page > availableFrames ? availableFrames : page;
    for (unsigned int i = 0; i < reuseFrames; i++) {
        FrameTableItem *fti = getAvailableFrame();
        fti->updateFrame(fti->getFrameAddress(), pages[i], pid);
        prti->updatePageTable(pages[i], fti->getFrameAddress());
        mPageTable->insertPage(pages[i], pid, fti->getFrameAddress());
    }
    page -= reuseFrames;

    // 对于剩余需要分配的内存，分成两部分（新分配／换页）
    // 对于需要新分配内存且未占用的帧，先分配，再插入页
    unsigned int needToAllocFrames = getNeedAllocFrameCount();
    unsigned long allocFrames = page > needToAllocFrames ? needToAllocFrames : page;
    auto *space = new char[allocFrames << BIT];
    for (unsigned int i = 0; i < allocFrames; i++) {
        FrameTableItem *fti = getNeedAllocFrame();
        fti->updateFrame((unsigned long)space + (i << BIT), pages[i + reuseFrames], pid);
        prti->updatePageTable(pages[i + reuseFrames], fti->getFrameAddress());
        mPageTable->insertPage(pages[i + reuseFrames], pid, fti->getFrameAddress());
    }
    page -= allocFrames;

    // 剩余部分，需要由 LRU 换页调出
    for (unsigned int i = 0; i < page; i++) {
        mPageTable->insertPage(pages[i + reuseFrames + allocFrames], pid);
    }

    // 在堆上分配的常规内存，需要更新堆已使用的大小
    if (start == 0) {
        prti->setHeapUsedSize(prti->getHeapHeader() + size);
    }

    return virtualPointer;
}

void MMU::freeUserMemory(unsigned int pid, unsigned long logicalAddress, unsigned long size) {
    auto item = mProcessTable.find(pid);
    if (mProcessTable.end() == item)
        return;
    ProcessTableItem *prti = item->second;

    // 要释放的内存大于当前已分配的堆大小
    if (size > prti->getHeapUsedSize())
        size = prti->getHeapUsedSize();

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;

    // 要释放的是堆空间的末尾，只需要将该部分内存置为弃用
    if (logicalAddress + size >= prti->getHeapHeader()) {
        auto page = logicalAddress >> BIT;
        if (logicalAddress & ((1 << BIT) - 1))
            page++;
        auto endPage = (prti->getHeapHeader() - 1) >> BIT;
        for (unsigned long i = page; i <= endPage; i++) {
            if (prti->getPageTableItem(i) && prti->getPageTableItem(i)->isUsed()) {
                freeFrame(pid, i);
                PageTableItem *pti = mPageTable->getPageTableItem(i, pid);
                if (pti)
                    pti->clearUsed();
                prti->getPageTableItem(i)->clearUsed();
            }
        }
        return;
    }

    // 要释放的是中间页，需要计算释放区域所占的页号，并将其对应更新
    unsigned long startPage = logicalAddress >> BIT;
    if (logicalAddress & ((1 << BIT) - 1))
        startPage++;
    unsigned long endAddress = logicalAddress + size;
    unsigned long endPage = endAddress >> BIT;

    // 要释放的页包括从起始释放地址对应的下一页到结束地址对应的前一页
    for (unsigned long i = startPage; i < endPage; i++) {
        PageTableItem *prpti = prti->getPageTableItem(i);
        if (prpti && prpti->isUsed()) {
            freeFrame(pid, i);
            PageTableItem *pti = mPageTable->getPageTableItem(i, pid);
            if (pti)
                pti->clearUsed();
            prti->getPageTableItem(i)->clearUsed();
        }
    }

}

bool MMU::pageFault(unsigned int pid, unsigned long logicalPage) {
    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return false;
    PageTableItem *pti = prti->getPageTableItem(logicalPage);
    if (!pti)
        return false;
    if (pti->isInMemory())
        return true;

    if (pti->isSwapped()) {
        // 已经被调出到 Swap 空间的，需要通过换页换入内存
        return mPageTable->schedule(logicalPage, pid);
    } else {
        /* 该页从未申请过，因此需要分配或换页。此时可能读取到各种预想不到的数据
         * 此处可能会产生漏洞：如果进程通过 read 操作去读取尚未被分配的内存，则 MMU 会
         * 通过换页来解决该操作的 pageFault，如果是 schedule 方式换页，则会令该进程读
         * 取到其他进程在该页的信息。因此在每个进程换页到 Swap 空间时，操作系统需要将该
         * 帧填充为其他数据。
         */
        return mPageTable->insertPage(logicalPage, pid);
    }
}

void MMU::freeFrame(unsigned int pid, unsigned long logicalPage) {
    for (FrameTableItem * fti : mFrameTable) {
        if (fti->isOccupiedBy(pid, logicalPage)) {
            fti->clear();
        }
    }
}

FrameTableItem* MMU::getFrame(unsigned long physicalAddress) {
    for (FrameTableItem *_fti: mFrameTable)
        if (_fti->getFrameAddress() == physicalAddress)
            return _fti;
    return nullptr;
}

// 此方法仅由页表满时的 LRU 调度调用，此时的 physicalAddress 已经在帧表中存在
void MMU::updateAll(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress) {
    FrameTableItem * fti = getFrame(physicalAddress);
    if (!fti)
        return;
    fti->updateFrame(physicalAddress, logicalPage, pid);
    // 操作系统 pid = 0 不置入进程表
    if (!pid)
        return;

    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return;
    prti->updatePageTable(logicalPage, physicalAddress);
}

unsigned int MMU::getAvailableFrameCount() {
    unsigned int count = 0;
    for (FrameTableItem *p : mFrameTable) {
        if (!p->isOccupied() && p->getFrameAddress() != 0) {
            count++;
        }
    }
    return count;
}

FrameTableItem* MMU::getAvailableFrame() {
    for (FrameTableItem *p : mFrameTable)
        if (!p->isOccupied() && p->getFrameAddress() != 0)
            return p;
    return nullptr;
}

unsigned int MMU::getAllocedFrameCount() {
    unsigned int count = 0;
    for (FrameTableItem *p : mFrameTable) {
        if (p->getFrameAddress() != 0) {
            count++;
        }
    }
    return count;
}

unsigned int MMU::getNeedAllocFrameCount() {
    unsigned int count = 0;
    for (FrameTableItem *p : mFrameTable) {
        if (p->getFrameAddress() == 0 && !p->isOccupied()) {
            count++;
        }
    }
    return count;
}

FrameTableItem* MMU::getNeedAllocFrame() {
    for (FrameTableItem *p : mFrameTable)
        if (p->getFrameAddress() == 0 && !p->isOccupied())
            return p;
    return nullptr;
}

ProcessTableItem* MMU::getProcess(unsigned int pid) {
    auto item = mProcessTable.find(pid);
    if (mProcessTable.end() == item)
        return nullptr;
    return item->second;
}

uint8_t MMU::readByte(unsigned int pid, unsigned long logicalAddress) {
    if (pid == 0) {
        void * physicalAddress = getPhysicalPointer(logicalAddress);
        return physicalAddress ? *(uint8_t *)physicalAddress : (uint8_t )0;
    }
    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return 0;
    return prti->readByte(logicalAddress);
}

uint32_t MMU::readHalfWord(unsigned int pid, unsigned long logicalAddress) {
    if (pid == 0) {
        uint32_t value = 0;
        for (unsigned long i = 0; i < 4; i++) {
            value <<= 8;
            value |= readByte(0, logicalAddress + 3 - i);
        }
        return value;
    }
    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return 0;
    return prti->readHalfWord(logicalAddress);
}

uint64_t MMU::readWord(unsigned int pid, unsigned long logicalAddress) {
    if (pid == 0) {
        uint64_t value = readHalfWord(pid, logicalAddress + 4);
        value <<= 32;
        value |= readHalfWord(pid, logicalAddress);
        return value;
    }
    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return 0;
    return prti->readWord(logicalAddress);
}

PageTableItem *MMU::getProcessPageTableItem(unsigned int pid, unsigned long logicalPage) {
    ProcessTableItem *prti = getProcess(pid);
    if (!prti)
        return nullptr;
    return prti->getPageTableItem(logicalPage);
}

unsigned long MMU::getAvailableSwapAddress() {
    // TODO
    return 0;
}

void* MMU::getPhysicalPointer(unsigned long logicalAddress, unsigned int pid) {
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long logicalPage = logicalAddress >> BIT;
    unsigned long offset = logicalAddress & ((1 << BIT) - 1);
    PageTableItem * pti = mPageTable->getPageTableItem(logicalPage, pid);
    if (pid != 0 && !pti) {
        pageFault(pid, logicalPage);
        pti = mPageTable->getPageTableItem(logicalPage, pid);
    }
    return pti ? (void *)(pti->getPhysicalAddress() + offset) : nullptr;
}

bool MMU::allocProcess(unsigned int pid, const std::string &path, unsigned long pageCount,
                       unsigned long stackPageCount) {
    // 已达到最大进程数量，无法分配
    if (mProcessTable.size() == Config::getInstance()->OS.MAXIMUM_TASKS)
        return false;

    // 计算该进程所需的最大维护空间
    unsigned long ptsize = sizeof(ProcessTableItem) + Config::getInstance()->OS.MAXIMUM_TASKS * sizeof(PageTableItem);

    // 寻找可用的进程表空间
    unsigned int number = 0;
    for (unsigned int i = 0; i < mProcessTableUsed.size(); i++) {
        if (!mProcessTableUsed[i]) {
            mProcessTableUsed[i] = true;
            number = i;
            break;
        }
    }

    // 从已分配的进程表内存中截取空间，该块内存连续
    char * pointer = processTableAvailable + number * ptsize;

    auto *prti = new(pointer) ProcessTableItem(pid, pageCount, stackPageCount);
    mProcessTable.insert(std::make_pair(pid, prti));
    prti->setProcessTableNumber(number);
    pointer += sizeof(ProcessTableItem);

    // 为每一虚拟页填写进程页表
    for (unsigned long i = 0; i < pageCount; i++) {
        auto *pti = new(pointer) PageTableItem(i, pid, 0);
        pointer += sizeof(PageTableItem);
        prti->insertPage(pti);
    }

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;

    // 为进程分配栈区
    allocUserMemory(pid, stackPageCount << BIT, (pageCount - stackPageCount) << BIT);

    // 从文件系统中读取可执行文件的长度、信息等
    // TODO
    unsigned long codeLength = 0;

    // 此时堆指针为 0，直接在整个内存区域分配代码段，因为在堆上分配，需要恢复堆大小
    allocUserMemory(pid, codeLength);
    prti->setHeapUsedSize(0);

    // 计算代码段的页数并将代码段拷贝入内存
    unsigned long leftSize = codeLength;
    unsigned long codePage = (codeLength >> BIT) + ((codeLength & ((1 << BIT) - 1)) == 0 ? 0 : 1);
    // auto handler = FS.openFile(path);
    for (unsigned long i = 0; i < codePage; i++) {
        void * dst = getPhysicalPointer(i << BIT, pid);
        unsigned long readSize = leftSize > PAGE ? PAGE : leftSize;
        // handler->seek(codeLength - leftSize);
        // FS.copy(dst, readSize, handler);
        leftSize -= readSize;
    }

    // 设置进程的堆信息
    prti->setHeapAddress(codeLength);

    return true;
}

bool MMU::freeProcess(unsigned int pid) {
    ProcessTableItem * prti = getProcess(pid);
    if (!prti)
        return false;
    unsigned int number = prti->getProcessTableNumber();
    if (number >= Config::getInstance()->OS.MAXIMUM_TASKS)
        return false;

    // 将该进程占用的系统内存释放
    mProcessTableUsed[number] = false;

    // 将该进程占用的所有虚拟帧释放
    for (unsigned long i = 0; i < prti->getPageCount(); i++) {
        PageTableItem *pti = prti->getPageTableItem(i);
        if (pti) {
            if (pti->isInMemory()) {
                pti->clearUsed();
                // 清除对应物理帧的占用
                FrameTableItem * fti = getFrame(pti->getPhysicalAddress());
                if (fti)
                    fti->clear();
            }
        }
    }

    // 将全局页表中与该进程相关的页全部刷新为未占用
    mPageTable->clearPageForPid(pid);
    mProcessTable.erase(pid);
    return true;
}

unsigned long MMU::getSystemUsedSize() {
    return mSystemUsedSize;
}

bool MMU::write(unsigned long logicalAddress, const void *src, unsigned long size, unsigned int pid) {
    if (pid == 0) {
        void * physicalAddress = getPhysicalPointer(logicalAddress);
        if (!physicalAddress)
            return false;
        memcpy(physicalAddress, src, size);
        return true;
    }
    ProcessTableItem *pti = getProcess(pid);
    if (!pti)
        return false;
    return pti->write(logicalAddress, src, size);
}

unsigned int MMU::getUsedFrameCount() {
    unsigned int count = 0;
    for (FrameTableItem * fti : mFrameTable)
        if (fti && fti->isOccupied()) {
            count++;
        }
    return count;
}