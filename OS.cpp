#pragma once

#include "OS.h"
#include <iostream>

void OSDebugStr(const char *tip, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << tip;
	}
}

void OSDebugStrn(const char * tip, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << tip << std::endl;
	}
}

void OSDebugInt(int num, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << num;
	}
}

void OSDebugIntn(int num, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << num << std::endl;
	}
}

void OSDebugFloat(F32 num, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << num;
	}
}

void OSDebugFloatn(F32 num, int level) {
	if (level <= DEBUG_LEVEL) {
		std::cout << num;
	}
}

void OSLogStr(const char * tip) {
#if LOG
	std::cout << tip << std::endl;
#endif // DEBUG
}
