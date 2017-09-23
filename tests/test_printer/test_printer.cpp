//
// Created by 王耀 on 2017/9/23.
//

#include "gtest/gtest.h"
#include "printer.h"
#include "fs.h"

TEST(TESTPrinter, TEST_PRINT) {
    FS::init("./fs");
    std::vector<std::string> ans = Printer::PRINT("-");
    std::string res = "";
    for (std::string a : ans) {
        res += a + "\n";
    }
    std::string standard = "       \n"
            "       \n"
            "       \n"
            "       \n"
            "  \"\"\"  \n"
            "       \n"
            "       \n"
            "       \n";
    EXPECT_EQ(res, standard);
    FS::destroy();
}