//
// Created by 王耀 on 2017/9/16.
//

#include "config.h"
#include "mmu.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

Config* Config::instance = nullptr;
bool Config::mShowErrorLog = true;

Config* Config::getInstance() {
    if (!instance)
        init();
    return instance;
}

bool Config::init(const std::string &path, const bool showErrorLog) {
    Config::mShowErrorLog = showErrorLog;
    delete instance;
    try {
        instance = new Config(path);
    } catch (std::bad_alloc& e) {
        if (Config::mShowErrorLog)
            std::cerr << "Alloc Error: " << e.what() << std::endl;
        return false;
    }
    return instance != nullptr && instance->check();
}

bool Config::check() {
    mValid = MMU::check();
    return mValid;
}

Config::Config(const std::string &path) {
    mConfigPath = path;
    if (!reload()) {
        if (Config::mShowErrorLog)
            std::cerr << "Using default config." << std::endl;
        loadDefault();
    }
}

/*
 * This method needs change when add/delete configurations items.
 */

bool Config::reload() {
    std::ifstream configFileStream(mConfigPath.c_str(), std::ifstream::in | std::ifstream::binary);
    if (configFileStream.fail()) {
        if (Config::mShowErrorLog)
            std::cerr << "Loading Config File Error: '" << mConfigPath << "'." << std::endl;
        return false;
    }
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::read_json(configFileStream, pt);
    } catch (boost::property_tree::ptree_error &e) {
        if (Config::mShowErrorLog)
            std::cerr << "Building JSON Tree Error: " << std::endl;
        return false;
    }
    try {
        boost::property_tree::ptree cpu = pt.get_child("CPU");
        boost::property_tree::ptree disk = pt.get_child("DISK");
        boost::property_tree::ptree mem = pt.get_child("MEM");
        boost::property_tree::ptree os = pt.get_child("OS");

        /* CPU configuration */
        CPU.CORE_NUM = cpu.get<unsigned int>("CORE_NUM");
        CPU.CPU_RATE = cpu.get<double>("CPU_RATE");

        /* DISK configuration */
        DISK.ROOT_PATH = disk.get<std::string>("ROOT_PATH");
        DISK.BLOCK_SIZE = disk.get<unsigned int>("BLOCK_SIZE");
        DISK.BLOCK_COUNT = disk.get<unsigned long>("BLOCK_COUNT");
        boost::property_tree::ptree disk_buffer = disk.get_child("BUFFER");
        DISK.BUFFER.DEFAULT_BLOCK = disk_buffer.get<unsigned int>("DEFAULT_BLOCK");
        DISK.BUFFER.MAX_BLOCK = disk_buffer.get<unsigned int>("MAX_BLOCK");
        DISK.BUFFER.MIN_BLOCK = disk_buffer.get<unsigned int>("MIN_BLOCK");
        DISK.DEFAULT_BLOCKS_PER_FILE = disk.get<unsigned int>("DEFAULT_BLOCKS_PER_FILE");

        /* MEM configuration */
        MEM.DEFAULT_CAPACITY = mem.get<unsigned long>("DEFAULT_CAPACITY");
        MEM.DEFAULT_PROCESS_PAGE = mem.get<unsigned long>("DEFAULT_PROCESS_PAGE");
        MEM.DEFAULT_STACK_PAGE = mem.get<unsigned long>("DEFAULT_STACK_PAGE");
        MEM.DEFAULT_PAGE_BIT = mem.get<unsigned char>("DEFAULT_PAGE_BIT");
        MEM.DEFAULT_PAGE_SIZE = (unsigned int)(1 << MEM.DEFAULT_PAGE_BIT);

        /* OS configuration */
        boost::property_tree::ptree os_mem = os.get_child("MEM");
        OS.MEM.DEFAULT_OS_USED_PAGE = os_mem.get<unsigned int>("DEFAULT_OS_USED_PAGE");
        OS.MEM.SWAP_PAGE = os_mem.get<unsigned long>("SWAP_PAGE");
        OS.MEM.SWAP_BLOCK = OS.MEM.SWAP_PAGE * (MEM.DEFAULT_PAGE_SIZE / DISK.BLOCK_SIZE);
        OS.MEM.SWAP_SIZE = OS.MEM.SWAP_PAGE << MEM.DEFAULT_PAGE_BIT;
        OS.BOOT_MEMORY_KB = os.get<unsigned int>("BOOT_MEMORY_KB");
        OS.MAXIMUM_TASKS = os.get<unsigned int>("MAXIMUM_TASKS");
        OS.MAXIMUM_TASK_PAGE = os.get<unsigned long>("MAXIMUM_TASK_PAGE");

    } catch (boost::property_tree::ptree_error &e) {
        if (Config::mShowErrorLog)
            std::cerr << "Parsing JSON Tree Error: " << e.what() << "." << std::endl;
        return false;
    }
    return true;
}

/*
 * This method needs change when add/modify/remove configuration items, or
 * you wanna adjust the default values for some configuration items.
 */

void Config::loadDefault() {
    CPU.CORE_NUM = _CPU_CORE_NUM;
    CPU.CPU_RATE = _CPU_CPU_RATE;
    DISK.ROOT_PATH = _DISK_ROOT_PATH;
    DISK.BLOCK_SIZE = _DISK_BLOCK_SIZE;
    DISK.BLOCK_COUNT = _DISK_BLOCK_COUNT;
    DISK.BUFFER.DEFAULT_BLOCK = _DISK_BUFFER_DEFAULT_BLOCK;
    DISK.BUFFER.MAX_BLOCK = _DISK_BUFFER_MAX_BLOCK;
    DISK.BUFFER.MIN_BLOCK = _DISK_BUFFER_MIN_BLOCK;
    DISK.DEFAULT_BLOCKS_PER_FILE = _DISK_DEFAULT_BLOCKS_PER_FILE;
    MEM.DEFAULT_CAPACITY = _MEM_DEFAULT_CAPACITY;
    MEM.DEFAULT_PROCESS_PAGE = _MEM_DEFAULT_PROCESS_PAGE;
    MEM.DEFAULT_STACK_PAGE = _MEM_DEFAULT_STACK_PAGE;
    MEM.DEFAULT_PAGE_BIT = _MEM_DEFAULT_PAGE_BIT;
    MEM.DEFAULT_PAGE_SIZE = _MEM_DEFAULT_PAGE_SIZE;
    OS.MEM.DEFAULT_OS_USED_PAGE = _OS_MEM_DEFAULT_OS_USED_PAGE;
    OS.MEM.SWAP_PAGE = _OS_MEM_SWAP_PAGE;
    OS.MEM.SWAP_BLOCK = _OS_MEM_SWAP_BLOCK;
    OS.MEM.SWAP_SIZE = _OS_MEM_SWAP_SIZE;
    OS.BOOT_MEMORY_KB = _OS_BOOT_MEMORY_KB;
    OS.MAXIMUM_TASKS = _OS_MAXIMUM_TASKS;
    OS.MAXIMUM_TASK_PAGE = _OS_MAXIMUM_TASK_PAGE;
}