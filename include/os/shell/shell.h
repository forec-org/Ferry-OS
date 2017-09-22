//
// Created by 王耀 on 2017/9/22.
//

#ifndef SDOS_SHELL_H
#define SDOS_SHELL_H

#include <string>
#include <utility>
#include <vector>

class Shell {
private:
    std::string mCurrentPath;
    std::string mBasePath;
public:
    explicit Shell(const std::string &basePath) {
        this->mBasePath = basePath;
    }
    void reload(const std::string &basePath) {
        mCurrentPath = "/";
        mBasePath = basePath;
    }
    void run();
    std::string generatePath(const std::string &path);
    std::string getOp(const std::string &command);
    std::vector<std::string> getArgs(const std::string &command);
};

#endif //SDOS_SHELL_H
