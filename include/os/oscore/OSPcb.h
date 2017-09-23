#pragma once

#include "OS.h"

class OSSem;

//---------------------------------------------------------------------------------------
//
//	进程控制块
//
//		内存中栈的结构如下：						----------
//									mStackBase->				低地址
//													----------
//														.
//														.
//														.
//													----------
//									mStackTop->		
//													----------
//														.
//														.
//														.
//													----------
//									mStackBottom->				高地址
//													----------
//
//---------------------------------------------------------------------------------------
class OSPcb {
private:
	UINT32		mStackBottom;					//堆栈底部地址
	UINT32		mStackTop;						//栈顶
	UINT32		mStackBase;						//栈基址

	STK_SIZE	mStackSize;						//堆栈段大小（字节）
	STK_SIZE	mStackUsed;						//当前使用的堆栈空间数量

	UINT16		mPid;							//进程id
		
	UINT16		mDelay;							//如果有延时记录延时时间（节拍数）

	BOOLEAN		mIsTimeOut;						//判断等待信号量是否超时

	UINT8		mState;							//任务状态

	UINT8		mPrio;							//任务优先级

	BOOLEAN		mToDel;							//标志任务需被删除

	UINT32		mSwitchCtr;						//记录任务被调入执行的次数
	UINT32		mTotalCycles;					//任务运行的总周期数

	OSSem		*mSemPtr;						//等待的信号量指针

	CHAR		mName[MAX_PCB_NAME];			//进程名字

public:
	OSPcb();									//创建时定好pid
	~OSPcb();

	void		init(const CHAR *name, UINT32 nameSize, UINT32 stackBase, UINT32 stackSize, UINT8 prio, UINT8 state = PROC_STATE_READY);

	void		clear();						//清空任务控制块

	void		setStackTop(UINT32 stackTop);
	void		setStackBase(UINT32 stackBase);
	UINT32		getStackTop();
	UINT32		getStackBase();

	void		setPid(UINT16 pid);				//设置任务pid			
	UINT16		getPid();						//获取任务pid

	UINT16		getDelay();						//返回延迟时间
	void		delay(UINT16 delay);			//进程延迟
	void		decDelay();						//延迟时间递减
	BOOLEAN		isTimeOut();					//判断是否超时
	void		setTimeOut(BOOLEAN isTimeOut);	//标志进程等待超时

	void		setState(UINT8 state);			//设置进程状态
	UINT8		getState();						//获取进程状态
	BOOLEAN		isWaitingEvent();				//判断是否正在等待事件发生
	BOOLEAN		isSuspend();					//判断是否挂起

	void		setPrio(UINT8 prio);			//设置任务优先级
	UINT8		getPrio();						//获取任务优先级

	void		setDel();						//标志任务需被删除

	void		incSwitchCtr();					//递增任务调度次数
	UINT32		getSwitchCtr();					//获取任务调度次数

	void		incTotalCycles();				//递增任务执行周期数
	UINT32		getTotalCycles();				//获取任务执行周期数

	BOOLEAN		isNew();						//判断是否是第一次进入CPU

	void		setSem(OSSem *psem);			//设置等待的信号量
	OSSem		*getSem();						//获取等待的信号量
	void		removeFromSem();				//将该任务从等待的信号量队列中删除

	char		*getName();						//获取进程名
};

