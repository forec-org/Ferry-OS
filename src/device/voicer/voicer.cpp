//
// Created by 王耀 on 2017/9/23.
//

#include <cstdlib>
#include "voicer.h"
#include "booter.h"

void Voicer::speak(const std::string &msg) {
    std::string command = "/usr/local/bin/espeak \"" + msg + "\" -s 120";
    if (system(command.c_str()) == -1) {
        BOOTER::ERROR("exception raised at voicer");
        return;
    }
}