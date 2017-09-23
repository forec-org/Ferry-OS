#pragma once

#include "SystemConfig.h"
#include "OS.h"
#include <thread>

class IntController;

//	ģ��Ӳ����ʱ��
class Timer {
	static Timer *gInstance;

private:
	bool			done;
	std::thread		mThread;

	IntController	*mIntCtrl;					//�жϿ���������

public:
	Timer();
	~Timer();

	void			init();

	void			exec();

	static Timer *getInstance();
	static void release();
};

