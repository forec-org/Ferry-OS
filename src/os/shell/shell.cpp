//
// Created by 王耀 on 2017/9/22.
//

#include <algorithm>
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include "mmu.h"
#include "fs.h"
#include "booter.h"
#include "shell.h"
#include <termios.h>
#include <unordered_set>
#include <atomic>
#include <thread>
#include <iomanip>
#include <cstdlib>
#include "compiler.h"
#include "OSCore.h"


int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    // 设置命令行为 raw mode
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO | ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    // 无阻塞读取字符
    // 如果无字符，返回-1
    character = fgetc(stdin);

    // 恢复原始的命令行设置
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

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
    BOOTER::ERROR("invalid argument count or format");
}

void Shell::no_such_file_or_directory(const string &path) {
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
        } else
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
        } while (!Op.empty());
    }
    args.erase(args.begin());
    return args;
}

std::string Shell::auto_complete() {
    int key;
    unsigned long cursor = 0;
    bool preComplete = false;
    int index = 0;
    string preCommand = "";
    bool anotherLine = false;
    std::string command;
    for (;;) {
        key = getkey();
        if (key != 9) {
            // 清除前次补全信息
            if (preComplete && anotherLine) {
                // 清除多出的一行
                cout << "\33[s\33[1B\33[0;H\33[K\33[u";
            }
            preComplete = false;
            anotherLine = true;
            preCommand = "";
            index = 0;
        }
        switch (key) {
            case 13:  // 回车
                return command;
            case 39:  // 方向键右
                if (cursor >= command.length())
                    break;
                cursor++;
                break;
            case 37:  // 方向键左
                if (cursor <= 0)
                    break;
                cursor--;
            case 8:   // 退格
                if (!command.empty()) {
                    command = command.substr(0, cursor - 1) + command.substr(cursor);
                    cursor--;
                }
                break;
            case 46:  // Delete
                if (!command.empty()) {
                    command = command.substr(0, cursor) + command.substr(cursor + 1);
                }
                break;
            case 9:   // Tab 补全
            {
                string Op = getOp(command);
                if (Op.empty()) {
                    command.append(1, '\t');
                    break;
                }
                vector<string> args = getArgs(command);
                vector<string> match;
                match.clear();
                if (args.empty()) {
                    // 只补全命令
                    for (string cm: commands) {
                        if (cm.find(Op) == 0)
                            match.emplace_back(cm);
                    }
                    if (match.empty())
                        break;
                    if (match.size() == 1) {
                        cout << match[0].substr(Op.length());
                        break;
                    }
                    // 可补全的数量超过 1 个
                    if (!preComplete) {
                        // 第一次补全
                        preCommand = Op; // 保存原始 command
                        cout << "\33[s"; // 保存光标位置
                        cout << "\33[1B"; // 光标下移一行
                        cout << "\33[0;H"; // 光标移动到该行开始
                        cout << match[0];
                        for (unsigned int i = 1; i < match.size(); i++)
                            cout << " " << match[i];
                        cout << "\33[u"; // 恢复光标位置
                        preComplete = true;
                        index = -1;
                        anotherLine = true;
                    } else {
                        // 对之前的补全结果不满意
                        index = (index + 1) % match.size();
                        int distance = command.length() - preCommand.length();
                        cout << "\33[" << distance << "D\33[K";  // 清除上次补全内容
                        cout << match[index].substr(Op.length());
                    }
                }
                break;
            }
            case -1:
                break;
            default:
                if (32 <= key && key <= 126)
                    command = command.append(1, (char) key);
                break;
        }
    }
}

void Shell::run() {

    cout << "Ferry OS [VERSION 1.0.1]" << endl;
    cout << "© 2017 Ferry OS Team. All rights reserved." << endl << endl;

    string command;
    string originOp;

    std::vector<string> stateMap;
    stateMap.emplace_back("READY");
    stateMap.emplace_back("RUNNING");
    stateMap.emplace_back("WAITING");
    stateMap.emplace_back("SUSPEND");
    stateMap.emplace_back("TERMINATED");
    stateMap.emplace_back("SWAPPED_READY");
    stateMap.emplace_back("SWAPPED_WAITING");
    stateMap.emplace_back("SWAPPED_SUSPEND");

    for (;;) {
        command.clear();
        std::cout << mCurrentPath << ">";
        getline(cin, command);
//        command = auto_complete();

        // 截取用户输入的最开始的命令
        string Op = getOp(command);
        originOp = Op;
        std::transform(Op.begin(), Op.end(), Op.begin(), ::tolower);
        vector<string> args = getArgs(command);

        if (Op == "touch") {
            // 创建文件
            if (args.size() != 1)
                invalid_args();
            else if (!FS::getInstance()->touchFile(appendPath(args[0], false)))
                BOOTER::ERROR("cannot create file: " + args[0]);
        } else if (Op == "mkdir") {
            // 创建目录
            if (args.size() != 1)
                invalid_args();
            else if (!FS::getInstance()->mkdir(appendPath(args[0], false)))
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
        } else if (Op == "watch") {
            // 查看输出
            bool cont = true;
            if (!args.empty() && args[1] == "-c") {
                cont = false;
            }

            unordered_set<unsigned int> pidList;
            for (string id: args) {
                try {
                    pidList.insert(std::stoi(id));
                } catch (std::invalid_argument &) {
                    continue;
                }
            }
            long long int lastPos = 0;
            std::ifstream reader;
            string lineLog;

            std::atomic<bool> quit(false);
            std::thread listenKey = std::thread([](std::atomic<bool> *q) {
                int key;
                while (!(*q)) {
                    key = getkey();
                    if (key != -1)
                        cout << "You input " << key << endl;
                    if (key == 27 || key == 'q' || key == 'Q' || key == 0x04) {// CTRL+D
                        *q = true;
                        break;
                    }
                }
            }, &quit);
            listenKey.detach();

            // 从上次看过处继续
            if (cont) {
                lastPos = lastPosition;
            }

            while (!quit) {
                reader.open(FS::getInstance()->getPath(".console"), ios::in);
                if (reader.fail()) {
                    BOOTER::ERROR("system console is locked");
                    break;
                }
                reader.seekg(0, ios::end);
                long long int temp = reader.tellg();
                reader.seekg(lastPos, ios::beg);
                lastPos = temp;
                if (pidList.empty()) {
                    while (!reader.eof()) {
                        std::getline(reader, lineLog);
                        if (!reader.eof()) {
                            if (lineLog.find(':') != string::npos)
                                lineLog = lineLog.substr(lineLog.find(':') + 1);
                            cout << lineLog << endl;
                        }
                    }
                } else {
                    while (!reader.eof()) {
                        std::getline(reader, lineLog);
                        if (!reader.eof()) {
                            if (lineLog.find(':') == string::npos)
                                continue;
                            string pid = lineLog.substr(0, lineLog.find(':'));
                            try {
                                int ipid = std::stoi(pid);
                                if (pidList.find(ipid) != pidList.end())
                                    cout << lineLog.substr(lineLog.find(':') + 1) << endl;
                            } catch (std::exception &e) {
                                continue;
                            }
                        }
                    }
                }
                lastPosition = temp;
                reader.close();
                BOOTER::wait(600);
            }
            quit = true;
        } else if (Op == "vi" || Op == "nano") {
            if (args.size() != 1) {
                invalid_args();
            } else {
                string relativePath = args[0];
                relativePath = FS::getInstance()->getPath(appendPath(relativePath, false));
                string cm = Op + " " + relativePath;
                system(cm.c_str());
            }
        } else if (Op == "cat") {
            if (args.size() != 1)
                invalid_args();
            else {
                std::ifstream reader;
                reader.open(FS::getInstance()->getPath(appendPath(args[0], false)), ios::in);
                if (reader.fail()) {
                    BOOTER::ERROR("cannot open file '" + args[0] + "'");
                } else {
                    string line;
                    while (!reader.eof()) {
                        std::getline(reader, line);
                        if (reader.eof())
                            break;
                        cout << line << endl;
                    }
                    reader.close();
                }
            }
        } else if (Op == "head") {
            if (args.size() != 1 && args.size() != 2)
                invalid_args();
            else {
                unsigned int arg_num = 10;
                if (args.size() == 2) {
                    try {
                        arg_num = std::stoi(args[1]);
                    } catch (std::invalid_argument &e) {
                        BOOTER::WARNING("your argument '" + args[1] + "' is invalid");
                        arg_num = 10;
                    }
                }
                std::ifstream reader;
                reader.open(FS::getInstance()->getPath(appendPath(args[0], false)), ios::in);
                if (reader.fail()) {
                    BOOTER::ERROR("cannot open file '" + args[0] + "'");
                } else {
                    string line;
                    unsigned int count = 0;
                    while (!reader.eof() && count < arg_num) {
                        std::getline(reader, line);
                        count++;
                        if (reader.eof())
                            break;
                        cout << line << endl;
                    }
                    reader.close();
                }
            }
        } else if (Op == "tail") {
            if (args.size() != 1 && args.size() != 2)
                invalid_args();
            else {
                unsigned int arg_num = 10;
                if (args.size() == 2) {
                    try {
                        arg_num = std::stoi(args[1]);
                    } catch (std::invalid_argument &e) {
                        BOOTER::WARNING("your argument '" + args[1] + "' is invalid");
                        arg_num = 10;
                    }
                }
                std::ifstream reader;
                reader.open(FS::getInstance()->getPath(appendPath(args[0], false)), ios::in | ios::ate);
                if (reader.fail()) {
                    BOOTER::ERROR("cannot open file '" + args[0] + "'");
                } else {
                    string line;
                    for (unsigned int i = 0; i <= arg_num; i++) {
                        while (reader.cur != reader.beg && reader.peek() != reader.widen('\n')) {
                            reader.seekg(-1, reader.cur);
                        }
                        if (reader.cur != reader.beg)
                            reader.seekg(-1, reader.cur);
                    }
                    if (reader.cur != reader.beg)
                        reader.seekg(2, reader.cur);
                    while (!reader.eof()) {
                        std::getline(reader, line);
                        if (reader.eof())
                            break;
                        cout << line << endl;
                    }
                    reader.close();
                }
            }
        } else if (Op == "top") {
            if (!args.empty())
                invalid_args();
            else {
                cout << "\33[?25l";  // 隐藏光标
                unsigned int time = 0;
                while (time++ < 10) {
                    cout << "\33[s";   // 保存光标位置

                    // 获得进程信息
                    std::vector<ProcState> info;
                    OSCore::getInstance()->getProcStates(info);

                    // 打印进程概括信息
                    cout << "Processes: " << info.size() << " total, "
                         << (info.size() == 0 ? 0 : 1) << " running, "
                         << (info.size() == 0 ? 0 : info.size() - 1) << " sleeping"
                         << endl;   // 1

                    // 打印 CPU 使用信息
                    float cpu_percent = OSCore::getInstance()->getCpuUsage();
                    cout << "CPU usage: " << setiosflags(ios::fixed) << setprecision(2)
                         << (cpu_percent * 100) << "%, "
                         << ((1.0 - cpu_percent) * 100) << "% idle"
                         << endl;   // 2

                    // 打印内存使用信息
                    unsigned long leftPage =
                            MMU::getInstance()->getCapacity() - MMU::getInstance()->getUsedFrameCount();
                    cout << "MemRegions: " << MMU::getInstance()->getCapacity() << " total, "
                         << leftPage << " Pages(" << (leftPage << Config::getInstance()->MEM.DEFAULT_PAGE_BIT)
                         << "B) resident"
                         << endl;   // 3

                    // 打印活动内存信息
                    cout << "MemActive: " << MMU::getInstance()->getAllocedFrameCount() << " hot, "
                         << (MMU::getInstance()->getCapacity() - MMU::getInstance()->getAllocedFrameCount())
                         << " unused"
                         << endl;   // 4

                    // 打印虚拟空间信息
                    cout << "VirtualMem: " << FS::getInstance()->getSwapUsedPage() << "P("
                         << (FS::getInstance()->getSwapUsedPage() << Config::getInstance()->MEM.DEFAULT_PAGE_BIT)
                         << "B) used, " << FS::getInstance()->getSwapAllocedPage() << "P(alloced)/ "
                         << FS::getInstance()->getSwapSpacePage() << "P(capability)"
                         << endl;   // 5

                    // 打印内存百分条
                    cout << "MEM: [";
                    unsigned int usedFrame = MMU::getInstance()->getUsedFrameCount();
                    unsigned long percent = usedFrame * 50 / MMU::getInstance()->getCapacity();
                    for (unsigned int i = 0; i < percent; i++)
                        cout << "█";
                    for (unsigned long i = percent; i < 50; i++)
                        cout << "|";
                    cout << "]"
                         << endl;   // 6

                    // 打印进程详细信息
                    BOOTER::YELLOW("Pid\t\tState\t\tSwitch\t\tCycles\t\tProcess");
                    cout << "\33[?25l";   // 隐藏光标
                    cout << endl;   // 7

                    for (ProcState pro: info) {
                        boost::filesystem::path ppath(pro.procName);
                        std::string filename = ppath.filename().string();
                        std::string extention = ppath.extension().string();
                        if (!extention.empty()) {
                            if (filename.find(extention) != std::string::npos)
                                filename = filename.substr(0, filename.find(extention));
                        }

                        unsigned int state = pro.state;
                        unsigned int count = 0;
                        while (state) {
                            count ++;
                            state >>= 1;
                        }

                        std::string stateStr = "UNKNOWN";
                        if (state < stateMap.size())
                            stateStr = stateMap[state];

                        cout << pro.pid << "\t\t"
                             << stateStr << "\t\t"
                             << pro.switchCnt << "\t\t"
                             << pro.totalCycles << "\t\t"
                             << filename
                             << endl;
                    }   // info.size()

                    cout << "\33[u"; // 恢复光标
                    cout << "\33[s"; // 保存光标

                    BOOTER::wait(1000); // 等待 1 s

                    for (unsigned int i = 0; i <= 7 + info.size(); i++) {
                        cout << "\33[K\33[1B";
                    }
                    cout << "\33[u"; // 恢复光标
                }
                cout << "\33[?25h";  // 显示光标
            }
        } else if (Op == "shutdown" || Op == "quit" || Op == "exit") {
            // 退出 shell
            break;
        } else if (Op.find(".fse") != std::string::npos) {
            // 可执行文件
            if (!args.empty())
                invalid_args();
            else {
                std::string relativePath = appendPath(originOp, false);
                if (!FS::getInstance()->isExist(relativePath)) {
                    no_such_file_or_directory(originOp);
                } else if (!FS::getInstance()->isFile(relativePath)) {
                    BOOTER::ERROR("'" + originOp + "' is a directory");
                } else {
                    unsigned char err = 1;
                    int pid = OSCore::getInstance()->processCreate(relativePath.c_str(), err);
                    if (err != 0) {
                        BOOTER::ERROR("file '" + originOp + "' is not a valid executable format");
                    } else {
                        BOOTER::SUCCESS("'" + originOp + "' is running as a new process (PID = " + std::to_string(pid) + ")");
                    }
                }
            }
        } else if (Op == "compile") {
            // 编译程序
            if (args.size() != 1 && args.size() != 2)
                invalid_args();
            else {
                std::string sourceFile = appendPath(args[0], false);
                int code = 1;
                if (args.size() == 1) {
                    code = FASM::getInstance()->Exec(sourceFile.c_str());
                } else {
                    std::string dstFile = appendPath(args[1], false);
                    code = FASM::getInstance()->Exec(sourceFile.c_str(), dstFile.c_str());
                }
                if (code == 0) {
                    BOOTER::SUCCESS("'" + args[0] + "' has been compiled");
                } else {
                    BOOTER::ERROR("lexical error in '" + args[0] + "'");
                }
            }
        } else if (Op == "kill") {
            // 结束进程
            if (args.size() != 1)
                invalid_args();
            else {
                int pid = -1;
                try {
                    pid = std::stoi(args[0]);
                    if (OSCore::getInstance()->processDelete(pid))
                        BOOTER::ERROR("cannot find process with Pid = " + std::to_string(pid));
                    else
                        BOOTER::SUCCESS("process (Pid = " + std::to_string(pid) + ") is terminated");
                } catch(std::invalid_argument &e) {
                    BOOTER::ERROR(args[0] + " is not a valid pid number");
                }
            }
        } else {
            // 找不到命令
            BOOTER::ERROR("command not found: " + Op);
        }

        cout.flush();
    }
}