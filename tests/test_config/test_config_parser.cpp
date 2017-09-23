//
// Created by 王耀 on 2017/9/16.
//

#include "gtest/gtest.h"
#include "config.h"

TEST(ConfigTest, CONFIG_INIT) {
    EXPECT_EQ(true, Config::init("../../config.json", false));
    EXPECT_NE(nullptr, Config::getInstance());
    EXPECT_EQ(true, Config::init("none_exist.json"));
}

TEST(ConfigTest, CONFIG_DEFAULT) {
    EXPECT_EQ(true, Config::init("none_exist.json", false));
    EXPECT_EQ(2, Config::getInstance()->CPU.CORE_NUM);
    EXPECT_EQ(5, Config::getInstance()->CPU.CPU_RATE);
    EXPECT_EQ(512, Config::getInstance()->DISK.BLOCK_SIZE);
    EXPECT_EQ(4, Config::getInstance()->DISK.DEFAULT_BLOCKS_PER_FILE);
    EXPECT_EQ(1, Config::getInstance()->MEM.DEFAULT_STACK_PAGE);
}

TEST(ConfigTest, CONFIG_RELOAD) {
    EXPECT_EQ(true, Config::init("../../config.json", true));
    EXPECT_NE(nullptr, Config::getInstance());
    Config::getInstance()->MEM.DEFAULT_STACK_PAGE = 1;
    EXPECT_EQ(1, Config::getInstance()->MEM.DEFAULT_STACK_PAGE);
    EXPECT_EQ(true, Config::getInstance()->reload());
    EXPECT_EQ(2, Config::getInstance()->MEM.DEFAULT_STACK_PAGE);
}

TEST(ConfigTest, JSON_TREE_PARSER) {
    EXPECT_EQ(true, Config::init("../../config.json", true));
    EXPECT_NE(nullptr, Config::getInstance());
    EXPECT_EQ(0, Config::getInstance()->DISK.BUFFER.MIN_BLOCK);
    EXPECT_EQ(128, Config::getInstance()->OS.MEM.DEFAULT_OS_USED_PAGE);
}

TEST(ConfigTest, VALID_CONFIG) {
    Config::init("", false);
    Config::getInstance()->OS.BOOT_MEMORY_KB = 0;
    EXPECT_EQ(false, Config::getInstance()->check());
}