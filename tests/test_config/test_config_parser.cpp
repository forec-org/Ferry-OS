//
// Created by 王耀 on 2017/9/16.
//

#include "gtest/gtest.h"
#include "config.h"

TEST(ConfigTest, ConfigInit) {
    EXPECT_EQ(true, Config::init("../../config.json", false));
    EXPECT_NE(nullptr, Config::getInstance());
    EXPECT_EQ(true, Config::init("none_exist.json"));
}

TEST(ConfigTest, ConfigDefault) {
    EXPECT_EQ(true, Config::init("none_exist.json", false));
    EXPECT_EQ(2, Config::getInstance()->CPU.CORE_NUM);
    EXPECT_EQ(1.0, Config::getInstance()->CPU.CPU_RATE);
    EXPECT_EQ(512, Config::getInstance()->DISK.BLOCK_SIZE);
    EXPECT_EQ(4, Config::getInstance()->DISK.DEFAULT_BLOCKS_PER_FILE);
    EXPECT_EQ(2048, Config::getInstance()->MEM.DEFAULT_STACK_SIZE);
}

TEST(ConfigTest, ConfigReload) {
    EXPECT_EQ(true, Config::init("../../config.json", false));
    EXPECT_NE(nullptr, Config::getInstance());
    Config::getInstance()->MEM.DEFAULT_STACK_SIZE = 1024;
    EXPECT_EQ(1024, Config::getInstance()->MEM.DEFAULT_STACK_SIZE);
    EXPECT_EQ(true, Config::getInstance()->reload());
    EXPECT_EQ(2048, Config::getInstance()->MEM.DEFAULT_STACK_SIZE);
}