#include "timer.h"
#include "interrupter.h"
#include <iostream>
#include "config.h"

Timer *Timer::gInstance = nullptr;

Timer::Timer() {
}

Timer::~Timer() {
	if (mThread.joinable()) {
		done = true;
		mThread.join();
	}
}

void Timer::init() {
	done = false;
	mIntCtrl = IntController::getInstance();
	mThread = std::thread([](Timer *t) {t->exec(); }, this);
}

void Timer::exec() {
	while (!done) {
		mIntCtrl->setIntReq(0);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / Config::getInstance()->OS.TICKS_PER_SEC));
	}
}

Timer * Timer::getInstance() {
	if (gInstance == nullptr) {
		gInstance = new Timer();
	}
	return gInstance;
}

void Timer::release() {
	delete gInstance;
	gInstance = nullptr;
}
