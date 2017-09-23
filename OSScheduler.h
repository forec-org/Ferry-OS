#pragma once

#include "OS.h"
#include <list>

//class OSPcbManager;
class OSPcb;

typedef std::list<OSPcb*> OSPcbList;

class OSScheduler {
	static OSScheduler *gInstance;
private:
	OSPcb			*mPcbCurr;							//当前正在执行的进程PCB
	OSPcbList		mWaitingList;						//等待进程列表
	OSPcbList		mReadyList;							//就绪态进程列表
	
	OSPcb			*mNextToRun;						//下一个需要执行的进程

	OSPcb			*mIdlePcb;							//空转进程PCB

	BOOLEAN			getNextToRun();						//获取下一个要调入CPU执行的进程

	OSScheduler();
public:
					
	~OSScheduler();

	void			init();								//初始化

	void			sched();							//调度程序

	OSPcb			*getCurrPcb();						//获取当前正在执行的进程控制块

	void			setIdlePcb(OSPcb *idlePcb);			//设置空转程序

	void			caculateDelay();					//计算等待队列中每个任务的等待时间

	BOOLEAN			toSwapOutCurr();					//判断当前任务是否应该被换出

	void			addReadyPcb(OSPcb *pcb);			//将进程加入就绪队列						
	void			addWaitingPcb(OSPcb *pcb);			//将进程加入等待队列
	void			removeFromReadyList(UINT16 pid);	//从就绪列表中删除某个pcb
	void			removeFromWaitingList(UINT16 pid);	//从等待列表中删除某个pcb

	OSPcb*			removeRunning();					//移除正在运行的进程

	UINT16			getReadyNum();						//获取就绪任务数量
	UINT16			getWaitingNum();					//获取等待任务数量

	static OSScheduler *getInstance();

	static void release();
};

