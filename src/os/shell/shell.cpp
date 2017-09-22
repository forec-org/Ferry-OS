//
// Created by 王耀 on 2017/9/22.
//

#include <iostream>
#include <boost/filesystem.hpp>
#include "mmu.h"
#include "fs.h"
#include "booter.h"
#include "shell.h"

using namespace std;
using namespace boost::filesystem;

std::string Shell::generatePath(const std::string &path) {
    std::string npath = path;
    while (npath.find('/') == 0) {
        npath = npath.substr(1);
    }
    return npath;
}

std::string Shell::getOp(const std::string &cm) {
    string command = cm;
    string Op;
    if (command.find(' ') == string::npos) {
        Op = command;
    } else {
        while (command.find(' ') == 0)
            command = command.substr(1);
        if (command.find(' ') == string::npos) {
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
    if (command.find(' ') == string::npos) {
        return args;
    } else {
        string Op = "";
        do {
            Op = getOp(command);
            args.emplace_back(Op);
            if (Op == "")
                break;
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
    cout << "©️ 2017 Ferry OS Team. All rights reserved." << endl << endl;

    string command;
    for(;;) {
        command.clear();
        std::cout << mBasePath << ">";
        getline(cin, command);

        cout << "Your command is: " << command << endl;

        // 截取用户输入的最开始的命令
        string Op = getOp(command);
        vector<string> args = getArgs(command);

        if (Op == "touch") {
            if (args.size() != 1)
                BOOTER::ERROR("invalid argument count.");
            else
                FS::getInstance()->touchFile(args[0]);
        } else {
            BOOTER::ERROR("command not found: " + Op);
        }

        cout.flush();
    }
}