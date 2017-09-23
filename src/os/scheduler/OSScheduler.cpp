#include "OSScheduler.h"
#include "OSPcbManager.h"
#include "SystemConfig.h"
#include "OSPcb.h"

OSScheduler* OSScheduler::gInstance = NULL;

BOOLEAN OSScheduler::getNextToRun() {
	if (mReadyList.empty()) {
		return FALSE;
	}
	mNextToRun = mReadyList.front();
	mReadyList.pop_front();
	return TRUE;
}

OSScheduler::OSScheduler() {
	mPcbCurr = NULL;
	mNextToRun = NULL;
}


OSScheduler::~OSScheduler() {

}

void OSScheduler::init() {

}

void OSScheduler::sched() {
	if (!getNextToRun()) {								//如果当前就绪队列中没有进程
		OSDebugStrn("sched do not find to run pcb", 3);
		if (mPcbCurr) {									//如果当前有进程执行，就继续执行
			mPcbCurr->delay(OS_TICKS_PER_SCHED);
			return;
		}
		mPcbCurr = mIdlePcb;							//否则启动空转进程
		mPcbCurr->setState(PROC_STATE_RUNNING);
		mPcbCurr->delay(OS_TICKS_PER_SCHED);
		return;
	}

	if (mPcbCurr) {										//如果当前有进程执行
		mPcbCurr->setState(PROC_STATE_READY);			
		if (mIdlePcb != mPcbCurr) {						//不是空转进程，就放回就绪队列
			mReadyList.push_back(mPcbCurr);
		}
		OSDebugStr("sched proc out, pid:", 3);
		OSDebugIntn(mPcbCurr->getPid(), 3);
	} 
	mPcbCurr = mNextToRun;								//替换为将要执行的进程
	mPcbCurr->setState(PROC_STATE_RUNNING);
	mPcbCurr->delay(OS_TICKS_PER_SCHED);
	mPcbCurr->incSwitchCtr();							//被调度次数递增
	OSDebugStr("sched proc in, pid:", 3);
	OSDebugIntn(mPcbCurr->getPid(), 3);
}

OSPcb * OSScheduler::getCurrPcb() {
	return mPcbCurr;
}

void OSScheduler::setIdlePcb(OSPcb * idlePcb) {
	mIdlePcb = idlePcb;
}

void OSScheduler::addReadyPcb(OSPcb * pcb) {
	mReadyList.push_back(pcb);
}

void OSScheduler::addWaitingPcb(OSPcb * pcb) {
	mWaitingList.push_back(pcb);
}

void OSScheduler::caculateDelay() {
	OSPcb *toSwap;
	auto begin = mWaitingList.begin();
	auto end = mWaitingList.end();
	for (auto wait = begin; wait != end;) {
		if ((*wait)->getDelay() == 0) {					//延迟时间为0说明是无限等待
			++wait;
			continue;
		}

		(*wait)->decDelay();

		if ((*wait)->getDelay() == 0) {					//超时则改变其状态并从等待队列中移到就绪队列
			if ((*wait)->isWaitingEvent()) {			//如果进程是等待某个事件超时
				(*wait)->setTimeOut(TRUE);				//设置超时标志
				(*wait)->removeFromSem();				//从对应信号量等待队列中删除该任务
				(*wait)->setSem(NULL);					//设置等待的信号量为空
														//加入就绪队列
				if ((*wait)->getState() == PROC_STATE_WAITING) {
					(*wait)->setState(PROC_STATE_READY);
				}
				else {
					(*wait)->setState(PROC_STATE_SWAPPED_READY);
				}
			}
			else if ((*wait)->isSuspend()) {			//如果任务是挂起超时,直接将其加入就绪队列
				if ((*wait)->getState() == PROC_STATE_SUSPEND) {
					(*wait)->setState(PROC_STATE_READY);
				}
				else {
					(*wait)->setState(PROC_STATE_SWAPPED_READY);
				}
			}
			toSwap = *wait;
			mReadyList.push_back(toSwap);
			wait = mWaitingList.erase(wait);
		}
		else {											//未超时则遍历下一个
			++wait;
		}
	}
}

BOOLEAN OSScheduler::toSwapOutCurr() {
	mPcbCurr->decDelay();
	if (mPcbCurr->getDelay() == 0) {
		return TRUE;
	}
	return FALSE;
}

void OSScheduler::removeFromReadyList(UINT16 pid) {
	for (auto iter = mReadyList.begin(); iter != mReadyList.end(); ++iter) {
		if ((*iter)->getPid() == pid) {
			mReadyList.erase(iter);
			return;
		}
	}
}

void OSScheduler::removeFromWaitingList(UINT16 pid) {
	for (auto iter = mWaitingList.begin(); iter != mWaitingList.end(); ++iter) {
		if ((*iter)->getPid() == pid) {
			mWaitingList.erase(iter);
			return;
		}
	}
}

OSPcb * OSScheduler::removeRunning() {
	OSPcb *toReturn = NULL;
	if (mPcbCurr != NULL) {
		toReturn = mPcbCurr;
		mPcbCurr = NULL;
	}
	return toReturn;
}

UINT16 OSScheduler::getReadyNum() {
	return (UINT16)mReadyList.size();
}

UINT16 OSScheduler::getWaitingNum() {
	return (UINT16)mWaitingList.size();
}

OSScheduler * OSScheduler::getInstance() {
	if (gInstance == NULL) {
		gInstance = new OSScheduler();
	}
	return gInstance;
}

void OSScheduler::release() {
	if (gInstance == NULL) {
		return;
	}
	delete gInstance;
	gInstance = NULL;
}
