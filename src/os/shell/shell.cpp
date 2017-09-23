//
// Created by 王耀 on 2017/9/22.
//

#include <algorithm>
#include <iostream>
#include <boost/filesystem.hpp>
#include "mmu.h"
#include "fs.h"
#include "booter.h"
#include "shell.h"

using namespace std;
using namespace boost::filesystem;

// 去掉路径前的 '/'
std::string Shell::generatePath(const std::string &path) {
    std::string npath = path;
    while (npath.find('/') == 0) {
        npath = npath.substr(1);
    }
    return npath;
}

void Shell::invalid_args() {
    BOOTER::ERROR("invalid argument count.");
}

void Shell::no_such_file_or_directory(const string& path) {
    BOOTER::ERROR("no such file or directory: " + path);
}

// 将当前路径和一个相对路径结合起来，如果参数为绝对路径，则返回该绝对路径。
// 默认为绝对路径
std::string Shell::appendPath(const std::string &path, bool absolute) {
    boost::filesystem::path bpath(mCurrentPath);

    // 清除路径两端空格
    string np = path;
    while (np.find(' ') == 0)
        np = np.substr(1);
    // 处理 . 和 ..
    string first = "";
    do {
        first = getOp(np, '/');
        np = np.substr(first.length());
        while (np.find('/') == 0)
            np = np.substr(1);
        while (np.find(' ') == 0)
            np = np.substr(1);

        if (first == ".")
            continue;
        else if (first == "..") {
            if (bpath.string() == "/")
                continue;
            bpath = bpath.parent_path();
        }
        else
            bpath.append(first);
    } while (first != "");

    if (absolute)
        return bpath.string();
    else
        return generatePath(bpath.string());
}

std::string Shell::getOp(const std::string &cm, char split) {
    string command = cm;
    string Op;
    if (command.find(split) == string::npos) {
        Op = command;
    } else {
        while (command.find(split) == 0)
            command = command.substr(1);
        if (command.find(split) == string::npos) {
            Op = command;
        } else {
            Op = command.substr(0, command.find(' '));
        }
    }
    return Op;
}

std::vector<std::string> Shell::getArgs(const std::string &cm) {
    string command = cm;
    vector<string> args;
    args.clear();
    if (command.find(' ') == string::npos || getOp(cm) == "") {
        return args;
    } else {
        string Op = "";
        do {
            Op = getOp(command);
            args.emplace_back(Op);
            if (Op == "") {
                args.pop_back();
                break;
            }
            while (command.find(' ') == 0)
                command = command.substr(1);
            command = command.substr(Op.length());
        } while(!Op.empty());
    }
    args.erase(args.begin());
    return args;
}

void Shell::run() {

    cout << "Ferry OS [VERSION 1.0.1]" << endl;
    cout << "© 2017 Ferry OS Team. All rights reserved." << endl << endl;

    string command;
    for(;;) {
        command.clear();
        std::cout << mCurrentPath << ">";
        getline(cin, command);

        // 截取用户输入的最开始的命令
        string Op = getOp(command);
        std::transform(Op.begin(), Op.end(), Op.begin(), ::tolower);
        vector<string> args = getArgs(command);

        if (Op == "touch") {
            // 创建文件
            if (args.size() != 1)
                invalid_args();
            else
                if (!FS::getInstance()->touchFile(appendPath(args[0], false)))
                    BOOTER::ERROR("cannot create file: " + args[0]);
        } else if (Op == "mkdir"){
            // 创建目录
            if (args.size() != 1)
                invalid_args();
            else
                if (!FS::getInstance()->mkdir(appendPath(args[0], false)))
                    BOOTER::ERROR("cannot create directory: " + args[0]);
        } else if (Op == "cd") {
            // 进入目录
            if (args.size() != 1)
                invalid_args();
            else {
                string npath = appendPath(args[0], false);
                if (FS::getInstance()->isDirectory(npath))
                    mCurrentPath = appendPath(args[0]);
                else if (FS::getInstance()->isFile(npath))
                    BOOTER::ERROR("'" + args[0] + "' is a file");
                else
                    no_such_file_or_directory(args[0]);
            }
        } else if (Op == "rm") {
            // 删除文件
            if (args.size() != 1)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isFile(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a directory");
                else if (!FS::getInstance()->rmFile(appendPath(args[0], false)))
                    BOOTER::ERROR("you have no permission to delete file: " + args[0]);
            }
        } else if (Op == "rmdir") {
            // 删除目录
            if (args.size() != 1)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isDirectory(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a file");
                else if (!FS::getInstance()->rmdir(appendPath(args[0], false)))
                    BOOTER::ERROR("you have no permission to delete directory: " + args[0]);
            }
        } else if (Op == "cp") {
            // 复制文件  cp from to
            if (args.size() != 2)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isFile(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a directory");
                else if (!FS::getInstance()->cpFile(appendPath(args[0], false), appendPath(args[1], false)))
                    BOOTER::ERROR("target file '" + args[1] + "' exists");
            }
        } else if (Op == "cpdir") {
            // 复制目录  cpdir from to
            if (args.size() != 2)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isDirectory(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a file");
                else if (!FS::getInstance()->cpDir(appendPath(args[0], false), appendPath(args[1], false)))
                    BOOTER::ERROR("you have no access to the target directory: " + args[1]);
            }
        } else if (Op == "mv") {
            // 移动文件  mv from to
            if (args.size() != 2)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isFile(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a directory");
                else if (!FS::getInstance()->mvFile(appendPath(args[0], false), appendPath(args[1], false)))
                    BOOTER::ERROR("target file '" + args[1] + "' exists");
            }
        } else if (Op == "mvdir") {
            // 移动目录 mvdir from to
            if (args.size() != 2)
                invalid_args();
            else {
                if (!FS::getInstance()->isExist(appendPath(args[0], false)))
                    no_such_file_or_directory(args[0]);
                else if (!FS::getInstance()->isDirectory(appendPath(args[0], false)))
                    BOOTER::ERROR("'" + args[0] + "' is a file");
                else if (!FS::getInstance()->mvDir(appendPath(args[0], false), appendPath(args[1], false)))
                    BOOTER::ERROR("you have no access to the target directory: " + args[1]);
            }
        } else if (Op == "ls") {
            // 查看目录
            if (args.size() != 0 && args.size() != 1)
                invalid_args();
            else {
                string lspath = "";
                if (args.size() == 1)
                    lspath = args[0];
                lspath = appendPath(lspath, false);
                if (!FS::getInstance()->isExist(lspath))
                    no_such_file_or_directory(args.size() == 1 ? args[0] : ".");
                else {
                    vector<string> names = FS::getInstance()->getFileNames(lspath);
                    // 先输出目录
                    for (string name: names) {
                        // 对该目录下每个文件获取文件名
                        string filePath = appendPath(name, false);
                        if (!FS::getInstance()->isDirectory(filePath))
                            continue;
                        BOOTER::YELLOW("DIR");
                        std::cout << "\t"
                                  << FS::getInstance()->getDirectorySize(filePath) << "\t"
                                  << FS::getInstance()->lastWriteTime(filePath) << "\t"
                                  << name << endl;
                    }
                    // 再输出文件
                    for (string name: names) {
                        string filePath = appendPath(name, false);
                        if (!FS::getInstance()->isFile(filePath))
                            continue;
                        std::cout << "FILE" << "\t"
                                  << FS::getInstance()->getFileSize(filePath) << "\t"
                                  << FS::getInstance()->lastWriteTime(filePath) << "\t"
                                  << name << endl;
                    }
                }
            }
        } else if (Op == "shutdown" || Op == "quit" || Op == "exit") {
            // 退出 shell
            break;
        } else {
            // 找不到命令
            BOOTER::ERROR("command not found: " + Op);
        }

        cout.flush();
    }
}