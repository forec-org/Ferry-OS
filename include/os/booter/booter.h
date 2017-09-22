//
// Created by 王耀 on 2017/9/15.
//

#ifndef SDOS_BOOTER_H
#define SDOS_BOOTER_H

#include <string>

class BOOTER {
private:
    static BOOTER * instance;
    BOOTER();
    ~BOOTER();
public:
    static void boot(const std::string& configPath = "./config.json", bool debug = false);
    static void shutdown();
    static void loading(unsigned int rount = 3, unsigned int count = 5, unsigned interval = 350, char code = '.');
    static void wait(unsigned int mseconds);
    static void ERROR(const std::string & msg);
    static void WARNING(const std::string & msg);
    static void BACKGROUND();
    static void CLEAR();
    static void SUCCESS(const std::string &msg = "SUCCESS");
    static void SHOW_CURSOR();
    static void LOGO();
};

#endif //SDOS_BOOTER_H
