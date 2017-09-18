//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_MEMORY_H
#define SDOS_MEMORY_H

#include <unordered_map>
#include "../../config/config.h"
#include "frame_table_item.h"
#include "page_table.h"
#include "process_table_item.h"

class MMU {
private:
    static unsigned long mCapacity;
    static long mSwapBaseAddress;
    static MMU *instance;
    static char *mBootAddress;
    char *mSystemAddress;
    PageTable *mPageTable;
    unsigned int mSystemUsedSize;
    std::unordered_map<long, FrameTableItem> mFrameTable;
    std::unordered_map<unsigned int, ProcessTableItem> mProcessTable;
    MMU();
    ~MMU();

public:
    static MMU *getInstance();
    static void init(unsigned long capacity = _MEM_DEFAULT_CAPACITY);
    static void destroy();
    static bool check();
    bool allocProcess(unsigned int pid,
                      unsigned long pageCount = _MEM_DEFAULT_PROCESS_PAGE,
                      unsigned long stackPageCount = _MEM_DEFAULT_STACK_PAGE);
    bool freeProcess(unsigned int pid);
    void *read(unsigned int pid, long logicalAddress);
    void *allocSystemMemory(unsigned long size);
    long allocUserMemory(unsigned int pid, unsigned long size);
};

#endif //SDOS_MEMORY_H
