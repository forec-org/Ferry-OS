//
// Created by 王耀 on 2017/9/23.
//

#include "fs.h"
#include <ctime>
#include <cstdlib>
#include <fstream>
#include "booter.h"
#include "printer.h"

std::string randStr(const int len) {
    std::string ans = "";
    for (int i = 0; i < len; ++i)
        ans.append(1, 'A' + rand() % 26);
    return ans;
}

std::vector<std::string> Printer::PRINT(const std::string &msg) {
    srand((unsigned int)time(nullptr));
    std::string randFilename = randStr(10);
    std::string absPath = FS::getInstance()->getPath(randFilename);
    std::string command = "/usr/local/bin/toilet \"" + msg + "\" > " + absPath;
    std::vector<std::string> result;
    result.clear();
    if (system(command.c_str()) == -1) {
        BOOTER::ERROR("exception raised at printer");
        return result;
    }
    std::ifstream reader;
    reader.open(absPath, std::ios::in);
    if (reader.fail()) {
        BOOTER::WARNING("no paper in printer");
        return result;
    }
    std::string line;
    while (!reader.eof()) {
        std::getline(reader, line);
        if (reader.eof())
            break;
        result.emplace_back(line);
    }
    FS::getInstance()->rmFile(randFilename);
    return result;
}