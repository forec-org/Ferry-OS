#include "OSPcb.h"
#include "OSSem.h"


OSPcb::OSPcb() {
	clear();
}

OSPcb::~OSPcb() {
}

void OSPcb::init(const CHAR *name, UINT32 nameSize, UINT32 stackBase, UINT32 stackSize, UINT8 prio, UINT8 state) {
	mStackBottom = stackSize - 1;
	mStackTop = mStackBottom;
	mStackBase = stackBase;
	mStackSize = stackSize;

	mPrio = prio;

	mState = state;

	memcpy(mName, name, nameSize);
	mName[nameSize] = '\0';
}

void OSPcb::clear() {
	mStackBase = 0;
	mStackBottom = 0;
	mStackTop = 0;

	mStackSize = 0;
	mStackUsed = 0;

	mDelay = 0;
	mIsTimeOut = FALSE;

	mState = PROC_STATE_TERMINATED;

	mPrio = 0;

	mToDel = FALSE;
	mSwitchCtr = 0;
	mTotalCycles = 0;

	mSemPtr = NULL;
}

void OSPcb::setStackTop(UINT32 stackTop) {
	mStackTop = stackTop;
}

void OSPcb::setStackBase(UINT32 stackBase) {
	mStackBase = stackBase;
}

UINT32 OSPcb::getStackTop() {
	return mStackTop;
}

UINT32 OSPcb::getStackBase() {
	return mStackBase;
}

void OSPcb::setPid(UINT16 pid) {
	mPid = pid;
}

UINT16 OSPcb::getPid() {
	return mPid;
}

UINT16 OSPcb::getDelay() {
	return mDelay;
}

void OSPcb::delay(UINT16 delay) {
	mDelay = delay;
}

void OSPcb::decDelay() {
	if (mDelay > 0) {
		mDelay--;
	}
}

BOOLEAN OSPcb::isTimeOut() {
	return mIsTimeOut;
}

void OSPcb::setTimeOut(BOOLEAN isTimeOut) {
	mIsTimeOut = isTimeOut;
}

void OSPcb::setState(UINT8 state) {
	mState = state;
}

UINT8 OSPcb::getState() {
	return mState;
}

BOOLEAN OSPcb::isWaitingEvent() {
	return (mState & PROC_STATE_WAITING_ANY) != PROC_STATE_READY;
}

BOOLEAN OSPcb::isSuspend() {
	return (mState & PROC_STATE_SUSPEND_ANY) != PROC_STATE_READY;
}

void OSPcb::setPrio(UINT8 prio) {
	mPrio = prio;
}

UINT8 OSPcb::getPrio() {
	return mPrio;
}

void OSPcb::setDel() {
	mToDel = TRUE;
}

void OSPcb::incSwitchCtr() {
	mSwitchCtr++;
}

UINT32 OSPcb::getSwitchCtr() {
	return mSwitchCtr;
}

void OSPcb::incTotalCycles() {
	mTotalCycles++;
}

UINT32 OSPcb::getTotalCycles() {
	return mTotalCycles;
}

BOOLEAN OSPcb::isNew() {
	return mStackBottom == mStackTop;
}

void OSPcb::setSem(OSSem * psem) {
	mSemPtr = psem;
}

OSSem * OSPcb::getSem() {
	return mSemPtr;
}

void OSPcb::removeFromSem() {
	if (mSemPtr) {
		mSemPtr->removeWaiting(this->mPid);
	}
}

char * OSPcb::getName() {
	return mName;
}
