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
    MMU::init();
    unsigned int BIT = Config::getInstance()->MEM.DEFAULT_PAGE_BIT;
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_PAGE;
    unsigned int pid1 = 1, pid2 = 2;
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid1, ""));
    EXPECT_TRUE(MMU::getInstance()->allocProcess(pid2, ""));
    unsigned long address1 = MMU::getInstance()->allocUserMemory(pid1, PAGE * 3);
    unsigned long address2 = MMU::getInstance()->allocUserMemory(pid2, PAGE * 2);
    EXPECT_TRUE(0, address1);
    EXPECT_TRUE(0, address2);
    unsigned long value = 0xAAAAAAAAAAAAAAAA;


}

