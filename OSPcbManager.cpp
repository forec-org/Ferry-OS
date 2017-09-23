#include "OSPcbManager.h"
#include "OSPcb.h"

OSPcbManager* OSPcbManager::gInstance = NULL;

OSPcbManager::OSPcbManager() {
	for (int index = 0; index < OS_MAX_PCB; ++index) {
		mCreated[index] = FALSE;
	}
	mProcCtr = 0;
}

OSPcbManager::~OSPcbManager() {
}

void OSPcbManager::init() {
	for (UINT16 index = 0; index < OS_MAX_PCB; ++index) {
		mPcbs[index].setPid(index);
		mFreeList.push_back(&mPcbs[index]);
	}
}

UINT16 OSPcbManager::getProcNum() {
	return mProcCtr;
}

BOOLEAN OSPcbManager::isCreated(UINT16 pid) {
	if (pid >= OS_MAX_PCB) {
		return FALSE;
	}
	return mCreated[pid];
}

OSPcbManager * OSPcbManager::getInstance() {
	if (gInstance == NULL) {
		gInstance = new OSPcbManager();
	}
	return gInstance;
}

void OSPcbManager::release() {
	if (gInstance == NULL) {
		return;
	}
	delete gInstance;
	gInstance = NULL;
}

OSPcb * OSPcbManager::getFreePCB(UINT8 & err) {
	if (mProcCtr == OS_MAX_PCB) {
		err = ERR_NO_FREE_PCB;
		return NULL;
	}

	OSPcb *free = mFreeList.front();			//获取队列头的空闲PCB，并从队列中删除
	mFreeList.pop_front();
	mCreated[free->getPid()] = TRUE;			//标志该PCB已被占用
	mProcCtr++;									//任务计数器增加
	err = ERR_NO_ERR;
	return free;
}

void OSPcbManager::returnPCB(OSPcb *pcb) {
	pcb->clear();								//清空pcb内容
	mFreeList.push_back(pcb);					//加入空闲列表
	mCreated[pcb->getPid()] = FALSE;			//标记未被占用
	mProcCtr--;									//计数器递减
}

OSPcb * OSPcbManager::getPCB(UINT16 pid) {
	if (isCreated(pid)) {
		return &mPcbs[pid];
	}
	return NULL;
}
