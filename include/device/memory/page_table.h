//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_PAGE_TABLE_H
#define SDOS_PAGE_TABLE_H

#include "page_table_item.h"
#include <unordered_map>
#include <utility>

typedef std::pair<unsigned long, unsigned int> kpair;

struct hash_func {
    size_t operator()(const kpair &pair) const {
        return pair.first * 9999 + pair.second;
    }
};

struct cmp_func {
    bool operator()(const kpair &k1, const kpair &k2) const {
        return k1.first == k2.first && k1.second == k2.second;
    }
};

class PageTable {
private:
    std::unordered_map<kpair, PageTableItem *, hash_func, cmp_func> mPageTableMap;
    PageTableItem *mLRUStackHead, *mLRUStackTail;   // LRU 双向链表
    // 页表在内存中存储于系统独占内存开始位置，mPageTableBaseAddress 用于存储页表内动态分配的页表项内存开始地址
    char * mPageTableBaseAddress;
    unsigned long mCapacity;
    unsigned int mUsed;           // 全局内存已使用页数
    unsigned int mAlloced;        // 全局内存已分配页数
public:
    explicit PageTable(unsigned long capacity, char *baseAddress);
    bool isFull();

    // 向页表中申请页，调用此方法时 MMU 找不到可用帧，物理内存已满，需由页表按 LRU 调度
    bool insertPage(unsigned long logicalPage, unsigned int pid);
    // MMU 已找到可用帧，为进程分配好内存，由页表更新即可
    bool insertPage(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress);
    // 按 LRU 策略换出
    bool schedule(unsigned long logicalPage, unsigned int pid);

    void deletePageTableItem(unsigned long logicalPage, unsigned int pid);
    void clearPageForPid(unsigned int pid);
    void stdErrPrint();
    PageTableItem *getPageTableItem(unsigned long logicalPage, unsigned int pid);
};

#endif //SDOS_PAGE_TABLE_H
