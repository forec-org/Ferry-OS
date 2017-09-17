//
// Created by 王耀 on 2017/9/16.
//

#include "config.h"
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
        std::cerr << "Alloc error caught: " << e.what() << std::endl;
        return false;
    }
    return instance != nullptr;
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
            std::cerr << "Error happens when loading config file '" << mConfigPath << "'." << std::endl;
        return false;
    }
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::read_json(configFileStream, pt);
    } catch (boost::property_tree::ptree_error &e) {
        if (Config::mShowErrorLog)
            std::cerr << "Error happens when building JSON tree from config stream." << std::endl;
        return false;
    }
    try {
        boost::property_tree::ptree cpu = pt.get_child("CPU");
        boost::property_tree::ptree disk = pt.get_child("DISK");
        boost::property_tree::ptree mem = pt.get_child("MEM");

        /* CPU configuration */
        CPU.CORE_NUM = cpu.get<int>("CORE_NUM");
        CPU.CPU_RATE = cpu.get<double>("CPU_RATE");

        /* DISK configuration */
        DISK.BLOCK_SIZE = disk.get<int>("BLOCK_SIZE");
        DISK.DEFAULT_BLOCKS_PER_FILE = disk.get<int>("DEFAULT_BLOCKS_PER_FILE");

        /* MEM configuration */
        MEM.DEFAULT_STACK_SIZE = mem.get<int>("DEFAULT_STACK_SIZE");

    } catch (boost::property_tree::ptree_error &e) {
        if (Config::mShowErrorLog)
            std::cerr << "Error happens when parsing values from JSON tree: " << e.what() << "." << std::endl;
        return false;
    }
    return true;
}

/*
 * This method needs change when add/modify/remove configuration items, or
 * you wanna adjust the default values for some configuration items.
 */

void Config::loadDefault() {
    CPU.CORE_NUM = 2;
    CPU.CPU_RATE = 1.0;
    DISK.BLOCK_SIZE = 512;
    DISK.DEFAULT_BLOCKS_PER_FILE = 4;
    MEM.DEFAULT_STACK_SIZE = 2048;
}