//
// Created by 王耀 on 2017/9/20.
//

#include "gtest/gtest.h"
#include "mmu.h"
#include "fs.h"

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
    FS::init();
    MMU::init();

    unsigned long program_size = 1024;
    auto space = new char[program_size];
    // 创建用于测试的程序
    EXPECT_TRUE(FS::getInstance()->writeFile("program1", space, program_size));
    delete []space;

    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned int pid1 = 1;
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid1, "program1"));
    EXPECT_NE(nullptr, MMU::getInstance()->getProcess(pid1));

    // 分配了代码段 1024
    EXPECT_EQ(program_size, MMU::getInstance()->getProcess(pid1)->getHeapHeader());
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

    auto space = new char[PAGE * 2];
    memset(space, 0xA5, PAGE * 2);

    FS::init();
    FS::getInstance()->writeFile("program", space, PAGE * 2);

    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid1, "program"));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid2, "program"));

    // 分配进程后操作系统已占用的帧数
    unsigned int usedCount1 = MMU::getInstance()->getUsedFrameCount();

    unsigned long address1 = MMU::getInstance()->allocUserMemory(pid1, PAGE * 3);
    unsigned long address2 = MMU::getInstance()->allocUserMemory(pid2, PAGE * 2);

    // 分配 5 块用户内存后，已使用帧数应增加 5
    EXPECT_EQ(usedCount1 + 5, MMU::getInstance()->getUsedFrameCount());

    // 堆栈指针应有变化
    EXPECT_EQ(MMU::getInstance()->getProcess(pid1)->getHeapHeader(), address1 + PAGE * 3);
    EXPECT_EQ(MMU::getInstance()->getProcess(pid2)->getHeapHeader(), address2 + PAGE * 2);

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

    unsigned int usedCount1 = MMU::getInstance()->getUsedFrameCount();
    EXPECT_EQ(usedCount1, MMU::getInstance()->getAllocedFrameCount());
    EXPECT_EQ(CAPACITY - usedCount1, MMU::getInstance()->getNeedAllocFrameCount());
    unsigned long systemUsedSize = MMU::getInstance()->getSystemUsedSize();
    unsigned long leftSize = systemUsedSize & (PAGE - 1);
    unsigned long toAllocSize = leftSize + (PAGE * 4);
    MMU::getInstance()->allocSystemMemory(toAllocSize);
    EXPECT_EQ(MMU::getInstance()->getUsedFrameCount(), MMU::getInstance()->getAllocedFrameCount());
    EXPECT_EQ(systemUsedSize + leftSize + PAGE * 4, MMU::getInstance()->getSystemUsedSize());
    EXPECT_EQ(usedCount1 + 4 + (leftSize > 0), MMU::getInstance()->getUsedFrameCount());

    MMU::destroy();
}

TEST(MMUTest, MMU_SWAP) {
    // 修改配置以方便配置
    Config::init();
    Config::getInstance()->MEM.DEFAULT_CAPACITY = 64;
    Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE = 32;
    Config::getInstance()->OS.MAXIMUM_TASKS = 4;
    Config::getInstance()->OS.MAXIMUM_TASK_PAGE = 64;
    Config::getInstance()->OS.MEM.SWAP_PAGE = 4;
    Config::getInstance()->MEM.DEFAULT_STACK_PAGE = 1;

    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    auto space = new char[PAGE * 7];
    memset(space, 0xA5, PAGE * 7);

    FS::init("./fs", 4);
    // 创建模拟的程序
    EXPECT_TRUE(FS::getInstance()->writeFile("program", space, PAGE, 0, true));
    EXPECT_EQ(PAGE, FS::getInstance()->getFileSize("program"));

    MMU::init(64);

    /* 进程可使用的内存只有 32 页，创建 4 个进程，每个进程占用 8 页，之后需要换页
     * 由于进程代码段占用 1 页，堆栈段占用 1 页，因此需要堆上分配 6 页即可占满全部用户内存。
     * */

    EXPECT_TRUE(MMU::getInstance()->allocProcess(1, "program", 64, 1));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(2, "program", 64, 1));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(3, "program", 64, 1));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(4, "program", 64, 1));

    // 将操作系统独占内存填满
    unsigned long usedSize = MMU::getInstance()->getSystemUsedSize();
    unsigned long systemSize = Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE * PAGE;
    unsigned long virtualSystemAddress = MMU::getInstance()->allocSystemMemory(systemSize - usedSize);

    // 确保操作系统内存分配无误
    EXPECT_TRUE(MMU::getInstance()->write(virtualSystemAddress, space, PAGE, 0));
    EXPECT_EQ(0xA5A5A5A5, MMU::getInstance()->readHalfWord(0, virtualSystemAddress + 4));

    EXPECT_EQ(systemSize, MMU::getInstance()->getSystemUsedSize());

    // 确保当前帧表使用数量正确  32 + 2 * 4 = 40
    EXPECT_EQ(40, MMU::getInstance()->getUsedFrameCount());


    // 为四个进程各分配 6 页，此时整个系统内存空间满
    unsigned long address1 = MMU::getInstance()->allocUserMemory(1, PAGE * 6);
    unsigned long address2 = MMU::getInstance()->allocUserMemory(2, PAGE * 6);
    unsigned long address3 = MMU::getInstance()->allocUserMemory(3, PAGE * 6);
    unsigned long address4 = MMU::getInstance()->allocUserMemory(4, PAGE * 6);

    // 将全部用户堆栈空间填满为 0xA5，以验证交换后的数据相同。覆盖区域包括栈区。操作系统不会被置换，因此无需覆盖。
    EXPECT_TRUE(MMU::getInstance()->write(address1, space, PAGE * 6, 1));
    EXPECT_TRUE(MMU::getInstance()->write(address2, space, PAGE * 6, 2));
    EXPECT_TRUE(MMU::getInstance()->write(address3, space, PAGE * 6, 3));
    EXPECT_TRUE(MMU::getInstance()->write(address4, space, PAGE * 6, 4));

    // 将栈区域填充 0xA5
    EXPECT_TRUE(MMU::getInstance()->write(MMU::getInstance()->getProcess(1)->getStackBaseAdderss() + 1 - PAGE, space, PAGE, 1));
    EXPECT_TRUE(MMU::getInstance()->write(MMU::getInstance()->getProcess(2)->getStackBaseAdderss() + 1 - PAGE, space, PAGE, 2));
    EXPECT_TRUE(MMU::getInstance()->write(MMU::getInstance()->getProcess(3)->getStackBaseAdderss() + 1 - PAGE, space, PAGE, 3));
    EXPECT_TRUE(MMU::getInstance()->write(MMU::getInstance()->getProcess(4)->getStackBaseAdderss() + 1 - PAGE, space, PAGE, 4));

    // 确保帧表已满
    EXPECT_EQ(64, MMU::getInstance()->getUsedFrameCount());

    // 确保写入无误
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(1, address1));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(4, address4));

    // 再为 pid1 分配 2 页，将会从内存中置换出 2 页到交换空间
    unsigned long address1_1 = MMU::getInstance()->allocUserMemory(1, PAGE * 2);

    // 按规则应该在堆区域继续分配
    EXPECT_EQ(address1 + PAGE * 6, address1_1);

    // 帧表应该仍满
    EXPECT_EQ(64, MMU::getInstance()->getUsedFrameCount());

    // 交换空间大小应该为 2 页
    EXPECT_EQ(PAGE * 2, FS::getInstance()->getSwapFileSize());

    // 将新分配的两页置换为其他数据，保证内存中有 2 页数据开头为 0x00
    memset(space, 0x55, PAGE * 2);
    EXPECT_TRUE(MMU::getInstance()->write(address1_1, space, 2 * PAGE, 1));
    EXPECT_EQ(0x5555555555555555, MMU::getInstance()->readWord(1, address1_1));

    // 读取最早的 32 页，保证全部都为 0xA5
    for (unsigned long i = 0; i < 6; i++) {
        EXPECT_EQ(0xA5, MMU::getInstance()->readByte(1, address1 + i * PAGE));
        EXPECT_EQ(0xA5, MMU::getInstance()->readByte(2, address2 + i * PAGE));
        EXPECT_EQ(0xA5, MMU::getInstance()->readByte(3, address3 + i * PAGE));
        EXPECT_EQ(0xA5, MMU::getInstance()->readByte(4, address4 + i * PAGE));
    }

    // 读栈区域保证该部分也为 0xA5
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(1, MMU::getInstance()->getProcess(1)->getStackBaseAdderss() - PAGE + 1));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(2, MMU::getInstance()->getProcess(2)->getStackBaseAdderss() - PAGE + 1));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(3, MMU::getInstance()->getProcess(3)->getStackBaseAdderss() - PAGE + 1));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(4, MMU::getInstance()->getProcess(4)->getStackBaseAdderss() - PAGE + 1));

    // 帧表满
    EXPECT_EQ(64, MMU::getInstance()->getUsedFrameCount());

    // 对于一块未分配的内存，读取应当触发调页，导致某页被换出，并换入该页，由于换出的页会将原始的物理帧填充 0 ，因此读取到 0
    EXPECT_EQ(0, MMU::getInstance()->readByte(2, address2 + 7 * PAGE));

    // 此时 SWAP 空间应当为 3 * PAGE 大小
    EXPECT_EQ(3 * PAGE, FS::getInstance()->getSwapFileSize());

    // 按照 LRU 策略，被换出的是最久未被使用的 1、2、3 代码段
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(1, 0));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(2, 0));
    EXPECT_EQ(0xA5, MMU::getInstance()->readByte(3, 0));

    EXPECT_EQ(64, MMU::getInstance()->getUsedFrameCount());

    delete []space;
    MMU::destroy();
    FS::format();
}