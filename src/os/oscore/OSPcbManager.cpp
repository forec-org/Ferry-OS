#include "OSPcbManager.h"
#include "config.h"

OSPcbManager* OSPcbManager::gInstance = NULL;

OSPcbManager::OSPcbManager() {
	for (int index = 0; index < Config::getInstance()->OS.MAXIMUM_TASKS; ++index) {
		mCreated[index] = FALSE;
	}
	mProcCtr = 0;
}

OSPcbManager::~OSPcbManager() {
}

void OSPcbManager::init() {
	for (UINT16 index = 0; index < Config::getInstance()->OS.MAXIMUM_TASKS; ++index) {
		mPcbs[index].setPid(index + 1);
		mFreeList.push_back(&mPcbs[index]);
	}
}

void OSPcbManager::getProcStates(std::vector<ProcState>& info) {
	for (UINT32 index = 0; index < Config::getInstance()->OS.MAXIMUM_TASKS; ++index) {
		if (mCreated[index]) {
			ProcState state;
			state.pid = mPcbs[index].getPid();
			state.procName = mPcbs[index].getName();
			state.state = mPcbs[index].getState();
			state.switchCnt = mPcbs[index].getSwitchCtr();
			state.totalCycles = mPcbs[index].getTotalCycles();

			info.push_back(state);
		}
	}
}

UINT16 OSPcbManager::getProcNum() {
	return mProcCtr;
}

BOOLEAN OSPcbManager::isCreated(UINT16 pid) {
	if (pid > Config::getInstance()->OS.MAXIMUM_TASKS) {
		return FALSE;
	}
	return mCreated[pid - 1];
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
	if (mProcCtr == Config::getInstance()->OS.MAXIMUM_TASKS) {
		err = ERR_NO_FREE_PCB;
		return NULL;
	}

	OSPcb *free = mFreeList.front();			//获取队列头的空闲PCB，并从队列中删除
	mFreeList.pop_front();
	mCreated[free->getPid() - 1] = TRUE;		//标志该PCB已被占用
	mProcCtr++;									//任务计数器增加
	err = ERR_NO_ERR;
	return free;
}

void OSPcbManager::returnPCB(OSPcb *pcb) {
	pcb->clear();								//清空pcb内容
	mFreeList.push_back(pcb);					//加入空闲列表
	mCreated[pcb->getPid() - 1] = FALSE;		//标记未被占用
	mProcCtr--;									//计数器递减
}

OSPcb * OSPcbManager::getPCB(UINT16 pid) {
	if (isCreated(pid)) {
		return &mPcbs[pid - 1];
	}
	return NULL;
}
