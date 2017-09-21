//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_MEMORY_H
#define SDOS_MEMORY_H

#include <vector>
#include <unordered_map>
#include <stdint.h>
#include "../../config/config.h"
#include "frame_table_item.h"
#include "page_table.h"
#include "process_table_item.h"

class MMU {
private:
    static unsigned long mCapacity;    // 模拟内存大小
    static unsigned long mSwapBaseAddress;      // Swap 区域的基地址
    static MMU *instance;              // 单例
    static char *mBootAddress;         // BOOT 区域内存基地址
    char *mSystemAddress;              // 操作系统独占区域内存基地址
    char *processTableAvailable;       // 操作系统独占区域中进程表的内存空间
    unsigned long mSystemUsedSize;     // 操作系统已使用的内存大小
    unsigned long mHostedSize;         // 不可被释放的区域大小
    PageTable *mPageTable;             // 全局页表（系统独占内存区域）
    std::vector<bool> mProcessTableUsed;  // 标记 ProcessTable[i] 是否被使用
    std::vector<FrameTableItem*> mFrameTable;   // 全局帧表（BOOT区域）
    std::unordered_map<unsigned int, ProcessTableItem*> mProcessTable;  // 进程页表（系统独占内存区域）
    MMU();
    [[deprecated("Since MMU is an instance, we suggest the OS to free its memory.")]]
    ~MMU();

public:
    static MMU *getInstance();

    // 初始化 MMU、全局帧表、全局页表
    static void init(unsigned long capacity = _MEM_DEFAULT_CAPACITY);
    static void destroy();

    // 检查 Config 中的配置是否满足 MMU 执行需要
    static bool check();

    // 创建新进程时更新页表信息（需将代码段拷贝、堆栈区域分配内存，其他区域随动态需求分配）
    bool allocProcess(unsigned int pid,
                      const std::string &path,
                      unsigned long pageCount = _MEM_DEFAULT_PROCESS_PAGE,
                      unsigned long stackPageCount = _MEM_DEFAULT_STACK_PAGE);
    // 释放进程全部内存，并更新页表等
    bool freeProcess(unsigned int pid);

    // 根据 pid 获取进程表项
    ProcessTableItem *getProcess(unsigned int pid);

    // 根据页表获取该进程对应逻辑地址的字节/字
    uint8_t readByte(unsigned int pid, unsigned long logicalAddress);
    uint32_t readHalfWord(unsigned int pid, unsigned long logicalAddress);
    uint64_t readWord(unsigned int pid, unsigned long logicalAddress);
    bool write(unsigned long logicalAddress, const void * src, unsigned long size, unsigned int pid);

    // 缺页处理，包括和 Swap 空间的调页
    PageTableItem *getProcessPageTableItem(unsigned int pid, unsigned long logicalPage);
    bool pageFault(unsigned int pid, unsigned long logicalPage);

    // 更新 MMU 下除全局页表外其它表项（该方法由全局页表的 LRU 操作调用）
    void updateAll(unsigned long logicalPage, unsigned int pid, unsigned long physicalAddress);

    // 在系统独占区域分配系统内存，超出时返回空
    unsigned long allocSystemMemory(unsigned long size);
    [[deprecated("This method could cause unexpected address moving.")]]
    void freeSystemMemory(unsigned long logicalAddress, unsigned long size);
    unsigned long getSystemUsedSize();

    // 为进程分配堆空间
    unsigned long allocUserMemory(unsigned int pid, unsigned long size, unsigned long start = 0);
    void freeUserMemory(unsigned int pid, unsigned long logicalAddress, unsigned long size);

    // 获取一块可用帧
    FrameTableItem *getFreeFrame();

    FrameTableItem *getFrame(unsigned long physicalAddress);

    // 释放帧表中的一帧
    void freeFrame(unsigned int pid, unsigned long logicalPage);

    // 获取交换空间中一块可用的页
    unsigned long getAvailableSwapAddress();
    void *getPhysicalPointer(unsigned long logicalAddress, unsigned int pid = 0);

    /*
     * 以下方法反馈 MMU 信息，帮助 OS 调试回显
     */

    // 获取已经分配实际内存，且可用的帧数
    unsigned int getAvailableFrameCount();
    FrameTableItem *getAvailableFrame();

    // 获取已经分配实际内存的帧数
    unsigned int getAllocedFrameCount();

    // 获取尚未分配实际内存，但未被占用的帧数
    unsigned int getNeedAllocFrameCount();
    FrameTableItem *getNeedAllocFrame();

    // 获取已占用的帧数
    unsigned int getUsedFrameCount();
};

#endif //SDOS_MEMORY_H
