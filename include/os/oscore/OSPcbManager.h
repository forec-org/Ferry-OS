#pragma once

#include "OS.h"
#include "SystemConfig.h"
#include "OSPcb.h"
#include <list>
#include <vector>

typedef std::list<OSPcb*> OSPcbList;

class OSPcbManager {
	static OSPcbManager *gInstance;

private:
	OSPcbList			mFreeList;						//空闲PCB列表
	BOOLEAN				mCreated[OS_MAX_PCB];			//PCB分配情况
	OSPcb				mPcbs[OS_MAX_PCB];				//PCB列表
	UINT16				mProcCtr;						//已创建任务计数

	OSPcbManager();
public:
	
	~OSPcbManager();

	void				init();							//初始化空闲PCB列表

	void				getProcStates(std::vector<ProcState> &info);
														//获取每个进程的状态

	OSPcb				*getFreePCB(UINT8 &err);		//获取空闲pcb

	void				returnPCB(OSPcb *pcb);			//任务删除后返还PCB

	OSPcb				*getPCB(UINT16 pid);			//获取已创建任务的PCB

	UINT16				getProcNum();					//获取当前任务数

	BOOLEAN				isCreated(UINT16 pid);			//判断任务是否被创建

	static OSPcbManager *getInstance();					//获取实例

	static void			release();						//删除实例
};

