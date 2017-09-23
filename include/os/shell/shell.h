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
    std::vector<std::string> commands;
public:
    explicit Shell(const std::string &basePath) {
        commands.clear();
        this->mBasePath = basePath;
        mCurrentPath = "/";
        commands.emplace_back("touch");
        commands.emplace_back("mkdir");
        commands.emplace_back("cp");
        commands.emplace_back("cpdir");
        commands.emplace_back("mv");
        commands.emplace_back("mvdir");
        commands.emplace_back("rm");
        commands.emplace_back("rmdir");
        commands.emplace_back("ls");
        commands.emplace_back("watch");
        commands.emplace_back("top");
        commands.emplace_back("cd");
        commands.emplace_back("shutdown");
        commands.emplace_back("quit");
        commands.emplace_back("exit");
        commands.emplace_back("vi");
        commands.emplace_back("nano");
        commands.emplace_back("cat");
        commands.emplace_back("head");
        commands.emplace_back("tail");
    }
    void reload(const std::string &basePath) {
        mCurrentPath = "/";
        mBasePath = basePath;
    }
    void run();
    std::string auto_complete();
    void invalid_args();
    void no_such_file_or_directory(const std::string & path);
    std::string appendPath(const std::string &path, bool absolute = true);
    std::string generatePath(const std::string &path);
    std::string getOp(const std::string &command, char split = ' ');
    std::vector<std::string> getArgs(const std::string &command);
};

#endif //SDOS_SHELL_H
