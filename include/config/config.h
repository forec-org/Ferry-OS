//
// Created by 王耀 on 2017/9/16.
//

#ifndef SDOS_CONFIG_H
#define SDOS_CONFIG_H

#include <string>

class Config {
private:
    std::string mConfigPath;
    static bool mShowErrorLog;
    static Config *instance;

    explicit Config(const std::string& path);
    void loadDefault();
public:
    static Config *getInstance();
    static bool init(const std::string &path = "./config.json", const bool showErrorLog = false);
    bool reload();

    struct CPU {
        int CORE_NUM;
        double CPU_RATE;  // GHz
    } CPU;

    struct DISK {
        int BLOCK_SIZE;  // Byte
        int DEFAULT_BLOCKS_PER_FILE;
    } DISK;

    struct MEM {
        int DEFAULT_STACK_SIZE;  // Byte
    } MEM;
};

#endif //SDOS_CONFIG_H
