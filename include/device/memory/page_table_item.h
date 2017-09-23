//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PAGE_TABLE_ITEM_H
#define SDOS_PAGE_TABLE_ITEM_H

#include <utility>

class PageTableItem {
private:
    unsigned long mLogicalPage;
    unsigned long mPhysicalAddress;
    unsigned long mSwapPage;
    unsigned int mPid;
    bool mUsed;
    bool mHosted;
    bool mSwapped;
    bool mHaveSwapped;
    bool mChanged;
    bool mInMemory;
    bool mSystem;
public:
    PageTableItem *pre, *next;
    explicit PageTableItem(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress);

    bool isInMemory() { return mInMemory; }
    bool isChanged() { return mChanged; }
    bool isSwapped() { return mSwapped; }
    bool isSystem() { return mSystem; }
    bool isHosted() { return mHosted; }
    bool isUsed() { return mUsed; }
    bool hasSwapped() { return mHaveSwapped; }
    void setSystem() { mSystem = true; }
    void setChanged() { mChanged = true; }
    void setSwapped() { mSwapped = true; }
    void setInMemory() { mInMemory = true; }
    void setHosted() { mHosted = true; }
    void setUsed() { mUsed = true; }
    void clearChanged() { mChanged = true; }
    void clearHosted() { mHosted = false; }
    void clearUsed() { mUsed = false; }

    bool swapIntoMemory();
    bool swapOutMemory();

    bool operator==(const PageTableItem &);

    void setPhysicalAddress(unsigned long physicalAddress);
    unsigned long getPhysicalAddress();

    void setLogicalPage(unsigned long logicalPage);
    unsigned long getLogicalPage();
    unsigned long getSwapPage();

    unsigned int getPid();
    void setPid(unsigned int pid);

    std::pair<long, unsigned int> getID();
    void reset();
};

#endif //SDOS_PAGE_TABLE_ITEM_H
