//
// Created by 王耀 on 2017/9/21.
//

#include "gtest/gtest.h"
#include "config.h"
#include "fs.h"

TEST(FSTest, FS_INIT) {
    std::string path = "./fs1";
    FS::init(path);

    // 验证文件系统根路径存在
    EXPECT_EQ(path, FS::getInstance()->getBasePath());
    EXPECT_TRUE(boost::filesystem::exists(path));
    EXPECT_TRUE(boost::filesystem::is_directory(path));

    // 验证使用 FS 读取的数据
    EXPECT_TRUE(FS::getInstance()->isExist(".swap"));
    EXPECT_TRUE(FS::getInstance()->isFile(".swap"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize(".swap"));

    FS::format();
}

TEST(FSTest, FS_FILE_DIRECTORY) {
    std::string path = "./fs2";
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

    FS::format();
}

TEST(FSTest, FS_READ_WRITE) {
    std::string path = "./fs3";
    FS::init();

    EXPECT_TRUE(FS::getInstance()->mkdir("home/rw"));
    EXPECT_TRUE(FS::getInstance()->touchFile("home/rw/test.dat"));
    EXPECT_EQ(0, FS::getInstance()->getFileSize("home/rw/test.dat"));

    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    auto space = new char[PAGE];
    memset(space, 0xA5, PAGE);

    EXPECT_TRUE(FS::getInstance()->writeFile("home/rw/test.dat", space, PAGE));
    EXPECT_EQ(PAGE, FS::getInstance()->getFileSize("home/rw/test.dat"));

    EXPECT_TRUE(FS::getInstance()->mkdir("home/rw/child"));
    EXPECT_TRUE(FS::getInstance()->isDirectory("home/rw/child"));
    EXPECT_TRUE(FS::getInstance()->mvFile("home/rw/test.dat", "home/rw/child/test.dat"));
    EXPECT_EQ(PAGE, FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    EXPECT_TRUE(FS::getInstance()->writeFile("home/rw/child/test.dat", space, PAGE >> 1, 0, true));
    EXPECT_EQ(PAGE >> 1, FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    EXPECT_TRUE(FS::getInstance()->appendFile("home/rw/child/test.dat", space, PAGE));
    EXPECT_EQ(PAGE + (PAGE >> 1), FS::getInstance()->getFileSize("home/rw/child/test.dat"));

    memset(space, 0, PAGE);

    EXPECT_EQ('\xA5', FS::getInstance()->readFileByte("home/rw/child/test.dat", 0));
    EXPECT_TRUE(FS::getInstance()->readFile("home/rw/child/test.dat", space, PAGE, PAGE >> 1));
    EXPECT_EQ('\xA5', space[PAGE-1]);
    EXPECT_TRUE(FS::getInstance()->rmdir("home"));

    delete []space;
    FS::format();
}

TEST(FSTest, FS_SWAP) {
    // 只设置 4 页交换空间简化测试
    FS::init("./fs4", 4);

    EXPECT_EQ(4, FS::getInstance()->getSwapSpacePage());
    EXPECT_EQ(0, FS::getInstance()->getSwapUsedPage());
    unsigned long swapPage1 = FS::getInstance()->allocSwapPage();
    unsigned long PAGE = Config::getInstance()->MEM.DEFAULT_PAGE_SIZE;
    auto space = new char[PAGE];
    memset(space, 0xA5, PAGE);

    // 分配的第一页必然为 0
    EXPECT_EQ(0, swapPage1);

    // 已分配一页
    EXPECT_EQ(PAGE, FS::getInstance()->getSwapFileSize());

    EXPECT_TRUE(FS::getInstance()->dumpPageIntoSwap(space, swapPage1));
    EXPECT_EQ(PAGE, FS::getInstance()->getSwapFileSize());

    unsigned long swapPage2 = FS::getInstance()->allocSwapPage();
    EXPECT_TRUE(FS::getInstance()->dumpPageIntoSwap(space, swapPage2));
    EXPECT_EQ(PAGE << 1, FS::getInstance()->getSwapFileSize());

    // 假设 page1 已无需再使用
    FS::getInstance()->clearSwapPage(swapPage1);

    // 已分配的第一块虽然不被占用但仍已分配
    EXPECT_EQ(PAGE << 1, FS::getInstance()->getSwapFileSize());

    swapPage1 = FS::getInstance()->allocSwapPage();
    EXPECT_EQ(0, swapPage1);

    memset(space, 0, PAGE);
    // 导入内存
    EXPECT_TRUE(FS::getInstance()->loadPageIntoMemory(space, swapPage2));
    EXPECT_EQ('\xA5', space[PAGE-1]);

    // 此时整个交换空间都是 '\xA5'
    EXPECT_EQ('\xA5', FS::getInstance()->readFileByte(".swap"));

    // 充满整个交换空间将导致最大页，可被检测到
    unsigned long maxPage = FS::getInstance()->getSwapSpacePage();
    for (unsigned long i = 2; i < maxPage; i++)
        FS::getInstance()->allocSwapPage();
    EXPECT_EQ(maxPage, FS::getInstance()->getSwapAllocedPage());
    EXPECT_EQ(maxPage, FS::getInstance()->allocSwapPage());

    FS::format();
}

TEST(FSTest, FS_FORMAT) {
    FS::init("./fs5");
    FS::format();
}