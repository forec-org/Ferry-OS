//
// Created by 王耀 on 2017/9/20.
//

#include "gtest/gtest.h"
#include "mmu.h"

TEST(MMUTest, MMU_INIT) {
    EXPECT_TRUE(Config::init("", false));
    EXPECT_TRUE(MMU::check());
}

TEST(MMUTest, MMU_ALLOC_SYSTEM_MEMORY) {
    MMU::init();
    unsigned long address1 = MMU::getInstance()->allocSystemMemory(4);
    EXPECT_EQ(address1, MMU::getInstance()->getSystemUsedSize() - 4);
    unsigned long address2 = MMU::getInstance()->allocSystemMemory(4);
    EXPECT_EQ(address1 + 4, address2);
    MMU::destroy();
}

TEST(MMUTest, MMU_MANAGE_BIG_SYSTEM_MEMORY) {
    MMU::init();
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned long address1 = MMU::getInstance()->allocSystemMemory(PAGE * 2);
    EXPECT_EQ(address1, MMU::getInstance()->getSystemUsedSize() - PAGE * 2);
    unsigned long address2 = MMU::getInstance()->allocSystemMemory(PAGE);
    EXPECT_EQ(address2, MMU::getInstance()->getSystemUsedSize() - PAGE);
    unsigned int value = 0xAAAAAAAA;
    EXPECT_TRUE(MMU::getInstance()->write(address2, (void *)&value, 4, 0));
    unsigned long address3 = MMU::getInstance()->allocSystemMemory(PAGE * 10);
    EXPECT_EQ(address3, MMU::getInstance()->getSystemUsedSize() - PAGE * 10);
    unsigned int value_ = MMU::getInstance()->readHalfWord(0, address2);
    EXPECT_EQ(value, value_);
    MMU::destroy();
}

TEST(MMUTest, MMU_USER_PROCESS) {
    MMU::init();
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned int pid1 = 1;
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid1, ""));
    EXPECT_NE(nullptr, MMU::getInstance()->getProcess(pid1));
    EXPECT_EQ(0, MMU::getInstance()->getProcess(pid1)->getHeapHeader());
    EXPECT_EQ(Config::getInstance()->MEM.DEFAULT_PROCESS_PAGE,
              MMU::getInstance()->getProcess(pid1)->getPageCount());
    EXPECT_EQ(MMU::getInstance()->getProcess(pid1)->getStackBaseAdderss(),
              (MMU::getInstance()->getProcess(pid1)->getPageCount() << BIT) - 1);
    EXPECT_TRUE(MMU::getInstance()->freeProcess(pid1));
    MMU::destroy();
}

TEST(MMUTest, MMU_USER_MEMORY) {
    Config::init("", false);
    // 保证测试不会因为内存过小失败
    Config::getInstance()->MEM.DEFAULT_CAPACITY = 1024;
    Config::getInstance()->OS.MAXIMUM_TASKS = 64;
    Config::getInstance()->MEM.DEFAULT_PAGE_BIT = 20;
    Config::getInstance()->MEM.DEFAULT_PAGE_SIZE = 1 << 20;

    MMU::init();
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned int pid1 = 1, pid2 = 2;

    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid1, ""));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid2, ""));

    // 分配进程后操作系统已占用的帧数
    unsigned int usedCount1 = MMU::getInstance()->getUsedFrameCount();

    unsigned long address1 = MMU::getInstance()->allocUserMemory(pid1, PAGE * 3);
    unsigned long address2 = MMU::getInstance()->allocUserMemory(pid2, PAGE * 2);

    // 分配 5 块用户内存后，已使用帧数应增加 5
    EXPECT_EQ(usedCount1 + 5, MMU::getInstance()->getUsedFrameCount());

    // 堆栈指针应有变化
    EXPECT_EQ(MMU::getInstance()->getProcess(pid1)->getHeapHeader(), address1 + PAGE * 3);
    EXPECT_EQ(MMU::getInstance()->getProcess(pid2)->getHeapHeader(), address2 + PAGE * 2);

    auto space = new char[PAGE * 2];
    memset(space, 0xA5, PAGE * 2);

    // 写操作
    EXPECT_TRUE(MMU::getInstance()->write(address1, space, PAGE * 2, pid1));

    // 在未分配内存情况下，多写一页，MMU 将自动分配该块内存
    EXPECT_TRUE(MMU::getInstance()->write(address2 + PAGE, space, PAGE * 2, pid2));

    // 由于 MMU 分配一块内存给 PID2，此时占用数量为 6
    EXPECT_EQ(usedCount1 + 6, MMU::getInstance()->getUsedFrameCount());

    // 读取写操作，确保读写数据一致性
    EXPECT_EQ(0xA5A5A5A5A5A5A5A5, MMU::getInstance()->readWord(pid1, address1 + PAGE));
    EXPECT_EQ(0xA5A5A5A5A5A5A5A5, MMU::getInstance()->readWord(pid2, address2 + PAGE));

    delete []space;

    // 确保在没有换页情况下，读操作不对页有影响
    EXPECT_EQ(usedCount1 + 6, MMU::getInstance()->getUsedFrameCount());

    // 释放部分用户内存，并测试是否从释放内存中分配新内存
    MMU::getInstance()->freeUserMemory(pid1, address1 + PAGE, PAGE);
    MMU::getInstance()->freeUserMemory(pid2, address2 + PAGE, PAGE);

    // 验证内存已释放
    EXPECT_EQ(usedCount1 + 4, MMU::getInstance()->getUsedFrameCount());

    // 有两帧已分配但被释放，可直接使用
    EXPECT_EQ(2, MMU::getInstance()->getAvailableFrameCount());

    unsigned long address1_1 = MMU::getInstance()->allocUserMemory(pid1, PAGE);
    unsigned long address2_1 = MMU::getInstance()->allocUserMemory(pid2, PAGE);

    EXPECT_EQ(address1 + PAGE, address1_1);
    EXPECT_EQ(address2 + PAGE, address2_1);

    // 再分配 2 页，使用量增加
    EXPECT_EQ(usedCount1 + 6, MMU::getInstance()->getUsedFrameCount());

    // 释放 MMU 由于写操作自动分配的内存
    MMU::getInstance()->freeUserMemory(pid2, address2 + PAGE * 2, PAGE);
    EXPECT_EQ(usedCount1 + 5, MMU::getInstance()->getUsedFrameCount());

    // 释放全部内存
    MMU::getInstance()->freeUserMemory(pid1, address1, PAGE * 3);
    MMU::getInstance()->freeUserMemory(pid2, address2, PAGE * 2);

    EXPECT_EQ(usedCount1, MMU::getInstance()->getUsedFrameCount());
    EXPECT_EQ(usedCount1 + 6, MMU::getInstance()->getAllocedFrameCount());

    MMU::destroy();
}

TEST(MMUTest, MMU_INFO) {
    MMU::init();
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    unsigned long CAPACITY = Config::getInstance()->MEM.DEFAULT_CAPACITY;
    unsigned long SYSTEM_PAGE = Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE;

    unsigned int usedCount1 = MMU::getInstance()->getUsedFrameCount();
    EXPECT_EQ(usedCount1, MMU::getInstance()->getAllocedFrameCount());
    EXPECT_EQ(CAPACITY - usedCount1, MMU::getInstance()->getNeedAllocFrameCount());
    unsigned long systemUsedSize = MMU::getInstance()->getSystemUsedSize();
    unsigned long leftSize = systemUsedSize & (PAGE - 1);
    unsigned long toAllocSize = leftSize + (PAGE * 4);
    unsigned long address = MMU::getInstance()->allocSystemMemory(toAllocSize);
    EXPECT_EQ(MMU::getInstance()->getUsedFrameCount(), MMU::getInstance()->getAllocedFrameCount());
    EXPECT_EQ(systemUsedSize + leftSize + PAGE * 4, MMU::getInstance()->getSystemUsedSize());
    EXPECT_EQ(usedCount1 + 4 + (leftSize > 0), MMU::getInstance()->getUsedFrameCount());

    unsigned int pid1 = 1, pid2 = 2;

    MMU::destroy();
}