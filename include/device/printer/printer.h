//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_PRINTER_H
#define SDOS_PRINTER_H

#include "../fs/fs.h"
#include <string>
#include <vector>

class Printer {
public:
    static std::vector<std::string> PRINT(const std::string & msg);
};

#endif //SDOS_PRINTER_H
