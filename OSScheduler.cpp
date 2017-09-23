#include "OSScheduler.h"
#include "OSPcbManager.h"
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
	if (!getNextToRun()) {								//�����ǰ����������û�н���
		OSDebugStrn("sched do not find to run pcb", 3);
		if (mPcbCurr) {									//�����ǰ�н���ִ�У��ͼ���ִ��
			mPcbCurr->delay(OS_TICKS_PER_SCHED);
			return;
		}
		mPcbCurr = mIdlePcb;							//����������ת����
		mPcbCurr->setState(PROC_STATE_RUNNING);
		mPcbCurr->delay(OS_TICKS_PER_SCHED);
		return;
	}

	if (mPcbCurr) {										//�����ǰ�н���ִ��
		mPcbCurr->setState(PROC_STATE_READY);			
		if (mIdlePcb != mPcbCurr) {						//���ǿ�ת���̣��ͷŻؾ�������
			mReadyList.push_back(mPcbCurr);
		}
		OSDebugStr("sched proc out, pid:", 3);
		OSDebugIntn(mPcbCurr->getPid(), 3);
	} 
	mPcbCurr = mNextToRun;								//�滻Ϊ��Ҫִ�еĽ���
	mPcbCurr->setState(PROC_STATE_RUNNING);
	mPcbCurr->delay(OS_TICKS_PER_SCHED);
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
		if ((*wait)->getDelay() == 0) {					//�ӳ�ʱ��Ϊ0˵�������޵ȴ�
			++wait;
			continue;
		}

		(*wait)->decDelay();

		if ((*wait)->getDelay() == 0) {					//��ʱ��ı���״̬���ӵȴ��������Ƶ���������
			if ((*wait)->isWaitingEvent()) {			//��������ǵȴ�ĳ���¼���ʱ
				(*wait)->setTimeOut(TRUE);				//���ó�ʱ��־
				(*wait)->removeFromSem();				//�Ӷ�Ӧ�ź����ȴ�������ɾ��������
				(*wait)->setSem(NULL);					//���õȴ����ź���Ϊ��
														//�����������
				if ((*wait)->getState() == PROC_STATE_WAITING) {
					(*wait)->setState(PROC_STATE_READY);
				}
				else {
					(*wait)->setState(PROC_STATE_SWAPPED_READY);
				}
			}
			else if ((*wait)->isSuspend()) {			//��������ǹ���ʱ,ֱ�ӽ�������������
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
		else {											//δ��ʱ�������һ��
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
