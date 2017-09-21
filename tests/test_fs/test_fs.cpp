//
// Created by 王耀 on 2017/9/21.
//

#include "gtest/gtest.h"
#include "config.h"
#include "fs.h"

TEST(FSTest, FS_INIT) {
    std::string path = "./fs";
    boost::filesystem::path bpath(path);
    FS::init(path);

    // 验证文件系统根路径存在
    EXPECT_EQ(path, FS::getInstance()->getBasePath());
    EXPECT_TRUE(boost::filesystem::exists(path));
    EXPECT_TRUE(boost::filesystem::is_directory(path));

    bpath.append(".swap");
    // 验证 SWAP 文件存在且大小为 0
    EXPECT_TRUE(boost::filesystem::exists(bpath));
    EXPECT_TRUE(boost::filesystem::is_regular_file(bpath));
    EXPECT_EQ(0, boost::filesystem::file_size(bpath));

    // 验证使用 FS 读取的数据与 BOOST 库读取相同
    EXPECT_TRUE(FS::getInstance()->isExist(".swap"));
    EXPECT_TRUE(FS::getInstance()->isFile(".swap"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize(".swap"));

    FS::destroy();
}

TEST(FSTest, FS_FILE_DIRECTORY) {
    std::string path = "./fs";
    FS::init(path);

    EXPECT_TRUE(FS::getInstance()->mkdir("home"));
    EXPECT_TRUE(FS::getInstance()->mkdir("forec/test/cpp"));
    EXPECT_TRUE(FS::getInstance()->isExist("forec/test"));
    EXPECT_TRUE(FS::getInstance()->isDirectory("forec/test/cpp"));

    EXPECT_TRUE(FS::getInstance()->touchFile("home/test.dat"));
    EXPECT_TRUE(FS::getInstance()->isExist("home/test.dat"));
    EXPECT_TRUE(FS::getInstance()->isFile("home/test.dat"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize("home/test.dat"));

    EXPECT_TRUE(FS::getInstance()->touchFile("forec/test/cpp/test.dat"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize("home/test.dat"));

    EXPECT_TRUE(FS::getInstance()->mvFile("home/test.dat", "forec/test/test.dat"));
    EXPECT_TRUE(FS::getInstance()->isExist("forec/test/test.dat"));
    EXPECT_TRUE(FS::getInstance()->isFile("forec/test/test.dat"));

    EXPECT_TRUE(FS::getInstance()->cpFile("forec/test/test.dat", "test.dat"));
    EXPECT_TRUE(FS::getInstance()->isExist("test.dat"));
    EXPECT_TRUE(FS::getInstance()->isFile("test.dat"));

    FS::destroy();
}

TEST(FSTest, FS_READ_WRITE) {
    std::string path = "./fs";
    FS::init();

    EXPECT_TRUE(FS::getInstance()->mkdir("home/rw"));
    EXPECT_TRUE(FS::getInstance()->touchFile("home/rw/test.dat"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize("home/rw/test.dat"));

    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    char *space = new char[PAGE];
    memset(space, 0xA5, PAGE);

    EXPECT_TRUE(FS::getInstance()->writeFile("home/rw/test.dat", space, PAGE));
    EXPECT_EQ(PAGE, FS::getInstance()->getFileSize("home/rw/test.dat"));

    EXPECT_TRUE(FS::getInstance()->mkdir("home/rw/child"));
    EXPECT_TRUE(FS::getInstance()->isDirectory("home/rw/child"));
    EXPECT_TRUE(FS::getInstance()->mvFile("home/rw/test.dat", "home/rw/child/test.dat"));
    EXPECT_EQ(PAGE, FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    EXPECT_TRUE(FS::getInstance()->writeFile("home/rw/child/test.dat", space, PAGE >> 1));
    EXPECT_EQ(PAGE >> 1, FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    EXPECT_TRUE(FS::getInstance()->appendFile("home/rw/child/test.dat", space, PAGE));
    EXPECT_EQ(PAGE + (PAGE >> 1), FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    memset(space, 0, PAGE);

    EXPECT_EQ(0xA5, FS::getInstance()->readFileByte("home/rw/child/test.dat", 0));
    EXPECT_TRUE(FS::getInstance()->readFile("home/rw/child/test.dat", space, PAGE, PAGE >> 1));
    EXPECT_EQ(0xA5, space[PAGE-1]);

    delete []space;
    FS::destroy();
}

TEST(FSTest, FS_SWAP) {
}

TEST(FSTest, FS_FORMAT) {
    FS::init("./fs");
    FS::format();
}