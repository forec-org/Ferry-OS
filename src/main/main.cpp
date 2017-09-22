//
// Created by 王耀 on 2017/9/15.
//

#include "booter.h"

int main() {
    BOOTER::boot("./config.json");
    BOOTER::shutdown();
    return 0;
}