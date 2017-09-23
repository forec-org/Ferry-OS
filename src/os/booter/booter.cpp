//
// Created by 王耀 on 2017/9/22.
//
#include <string>
#include "booter.h"
#include <cstdlib>
#include <iostream>
#include <boost/filesystem.hpp>
#include "config.h"
#include "fs.h"
#include "mmu.h"
#include "shell.h"
#include <thread>

using namespace std;
using namespace boost::filesystem;

BOOTER * BOOTER::instance = nullptr;

BOOTER::BOOTER() {}

BOOTER::~BOOTER() {}

void BOOTER::wait(unsigned int mseconds) {
    usleep(mseconds * 1000);
}

void BOOTER::CLEAR() {
    cout << "\033[0m";
    cout << "\033[?25h";
}

void BOOTER::BACKGROUND() {
    cout << "\33[?25l";  // 隐藏光标
}

void BOOTER::SHOW_CURSOR() {
    cout << "\33[?25h";  // 显示光标
}

void BOOTER::SUCCESS(const std::string &msg) {
    cout << "  \033[36m[" << msg << "]"; // 绿色成功
    CLEAR();
    cout << endl;
    BACKGROUND();
}

void BOOTER::ERROR(const std::string &msg) {
    cout << "\033[31mERROR: " << msg;    // 红色错误
    CLEAR();
    cout << endl;
}

void BOOTER::WARNING(const std::string &msg) {
    cout << "\033[33mWARNING: " << msg;  // 黄色警告
    CLEAR();
    cout << endl;
}

void BOOTER::RED(const std::string &msg) {
    cout << "\033[31m" << msg;    // 红色字体
    CLEAR();
}

void BOOTER::YELLOW(const std::string &msg) {
    cout << "\033[33m" << msg;  // 黄色字体
    CLEAR();
}

void BOOTER::loading(unsigned int round, unsigned int count, unsigned int interval, char code) {
    for (unsigned int i = 0; i < round; i++) {
        for (unsigned int j = 0; j < count; j++) {
            cout << code;
            for (unsigned int k = 0; k < count - j; k++)
                cout << " ";
            cout.flush();
            BOOTER::wait(interval);
            for (unsigned int k = 0; k < count - j; k++)
                cout << "\b";
            cout.flush();
        }
        for (unsigned int j = 0; j < count; j++)
            cout << "\b";
    }
    for (unsigned int i = 0; i < count; i++)
        cout << code;
    cout.flush();
}

void BOOTER::LOGO() {
    cout << "███████╗███████╗██████╗ ██████╗ ██╗   ██╗ ██████╗ ███████╗\n"
            "██╔════╝██╔════╝██╔══██╗██╔══██╗╚██╗ ██╔╝██╔═══██╗██╔════╝\n"
            "█████╗  █████╗  ██████╔╝██████╔╝ ╚████╔╝ ██║   ██║███████╗\n"
            "██╔══╝  ██╔══╝  ██╔══██╗██╔══██╗  ╚██╔╝  ██║   ██║╚════██║\n"
            "██║     ███████╗██║  ██║██║  ██║   ██║   ╚██████╔╝███████║\n"
            "╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝\n"
            "                                                          " << endl;
}

void BOOTER::boot(const std::string &configPath, bool debug) {
    system("clear");
    BACKGROUND();
    if (!exists(configPath)) {
        WARNING("CONFIG FILE '" + configPath + "' IS NOT EXIST!");
        BACKGROUND();
        cout << "TRY TO BOOT WITH DEFAULT CONFIG!" << endl << endl;
    }

    // 加载虚拟机配置
    cout << "LOADING VIRTUAL MACHINE CONFIG ";
    if (!Config::init(configPath, debug)) {
        ERROR("VIRTUAL MACHINE IS UNABLE TO BOOT WITH CONFIG FILE '" + configPath + "'!");
        BACKGROUND();
        WARNING("VIRTUAL MACHINE IS GOING TO SHUTDOWN ...");
        BACKGROUND();
        cout << endl;
        return;
    }
/*
    loading(3, 5, 100);
    SUCCESS();
    wait(400);
*/
    // 加载文件系统
    cout << "DETECTING FILE SYSTEM ";
    FS::init(Config::getInstance()->DISK.ROOT_PATH);
/*
    loading(1, 14, 120);
    SUCCESS();
    cout << "FINDING ONE HDD DISK: " << endl;
    cout << "/disk/sda:" << endl;
    cout << "    BLOCK SIZE: " << Config::getInstance()->DISK.BLOCK_SIZE << " Bytes" << endl;
    cout << "    BLOCK COUNT: " << Config::getInstance()->DISK.BLOCK_COUNT << " Blocks" << endl;
    cout << "    BLOCK BUFFER: " << Config::getInstance()->DISK.BUFFER.DEFAULT_BLOCK << " Blocks" << endl;
    cout << "    DISK SIZE: " << FS::getInstance()->getDirectorySize("") << " B(USED)/ " <<
         Config::getInstance()->DISK.BLOCK_SIZE * Config::getInstance()->DISK.BLOCK_COUNT << " B(TOTAL)" << endl << endl;
    wait(300);
*/
    // 加载内存
    cout << "VERIFYING MEMORY VALIDATION ";
    MMU::init(Config::getInstance()->MEM.DEFAULT_CAPACITY);

/*
    loading(1, 8, 100);
    SUCCESS();
    cout << "FINDING ONE RAM: " << endl;
    cout << "    RAM SIZE: " << (Config::getInstance()->MEM.DEFAULT_CAPACITY << Config::getInstance()->MEM.DEFAULT_PAGE_BIT) << " Bytes" << endl;
    cout << "    RAM RATE: 1600 MHz" << endl;
    cout << "    PAGE SIZE: " << Config::getInstance()->MEM.DEFAULT_PAGE_SIZE << " Bytes" << endl << endl;

    cout << "INITIALIZE FRAME TABLE ";
    loading(2, 7, 100);
    SUCCESS();

    cout << "DETECTING CPU INFO  ";
    loading(1, 10, 30);
    SUCCESS();
    cout << "FIND ONE PROCESSOR: " << endl;
    cout << "/proc/cpuinfo: " << endl;
    cout << "    CPU RATE: " << Config::getInstance()->CPU.CPU_RATE << " GHz" << endl;
    cout << "    CORE COUNT: " << Config::getInstance()->CPU.CORE_NUM << " CORE" <<  endl;
    cout << "    ARCHITECTURE: x86_f" << endl << endl;

    LOGO();
    cout << "LOADING Ferry OS  ";
    loading(1, 10, 150);
    SUCCESS();
*/

    // 创建 shell
    auto *shell = new Shell(Config::getInstance()->DISK.ROOT_PATH);

    // 启动 OS
    // std::thread osThread = std::thread([](OSCore *os){ os->run(); }, os);

/*
    loading(1, 10, 40, '>');
    loading(1, 20, 200, '>');
    loading(1, 10, 300, '>');
    loading(1, 30, 80, '>');
    cout << endl;
    system("clear");
*/

    CLEAR();
    // 启动 shell
    shell->run();
    delete shell;

}

void BOOTER::shutdown() {
    BACKGROUND();
    system("clear");

    LOGO();
    cout << endl;

    string s0 = "WAITING FOR ALL PROCESSES STOP ";
    cout << s0;
    loading(1, 50 - s0.length(), 40, '>');
    SUCCESS();

    string s1 = "CLEARING USER PROCESSES AND MEMORY ";
    cout << s1;
    loading(1, 50 - s1.length(), 70, '>');
    SUCCESS();

    string s2 = "CLEARING SYSTEM MEMORY ";
    cout << s2;
    loading(1, 50 - s2.length(), 45, '>');
    SUCCESS();

    string s3 = "SHUTING DOWN ";
    cout << s3;
    loading(1, 50 - s3.length(), 80, '>');
    SUCCESS();
    wait(800);
    cout << "GOOD BYE!" << endl;
    SHOW_CURSOR();
    CLEAR();
}