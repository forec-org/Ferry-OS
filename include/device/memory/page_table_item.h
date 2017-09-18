//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PAGE_TABLE_ITEM_H
#define SDOS_PAGE_TABLE_ITEM_H

#include <utility>

class PageTableItem {
private:
    long mLogicalAddress;
    long mPhysicalAddress;
    long mSwapAddress;
    unsigned int mPid;
    unsigned int used;   // LRU
    bool mSwapped;
    bool mChanged;
    bool mInMemory;
    bool mSystem;
public:
    PageTableItem *pre, *next;
    explicit PageTableItem(long logicalAddress, unsigned int pid, long physicalAddress);

    bool isInMemory() { return mInMemory; }
    bool isChanged() { return mChanged; }
    bool isSwapped() { return mSwapped; }
    bool isSystem() { return mSystem; }
    void setSystem() { mSystem = true; }

    bool swapIntoMemory();
    bool swapOutMemory();

    bool operator==(const PageTableItem &);

    void setPhysicalAddress(long physicalAddress);
    long getPhysicalAddress();

    void setLogicalAddress(long logicalAddress);
    long getLogicalAddress();

    unsigned int getPid();
    void setPid(unsigned int pid);

    std::pair<long, unsigned int> getID();
    void reset();

    void write(char *src, unsigned int size);
    char *read();
};

#endif //SDOS_PAGE_TABLE_ITEM_H
