#pragma once

#include <thread>

class IntController;

//	模拟硬件定时器
class Timer {
	static Timer *gInstance;

private:
	bool			done;
	std::thread		mThread;

	IntController	*mIntCtrl;					//中断控制器引用

public:
	Timer();
	~Timer();

	void			init();

	void			exec();

	static Timer *getInstance();
	static void release();
};

