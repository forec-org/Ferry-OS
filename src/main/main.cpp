//
// Created by 王耀 on 2017/9/15.
//

#include <iostream>
#include "config.h"

int main() {
    Config::init("../../config.json");
    std::cout << "CPU.CORE_NUM: " << Config::getInstance()->CPU.CORE_NUM << std::endl;
    return 0;
}