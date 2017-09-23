#include "OSSemManager.h"

OSSemManager *OSSemManager::gInstance = NULL;

OSSemManager::OSSemManager() {
}


OSSemManager::~OSSemManager() {
}

void OSSemManager::init() {
	for (UINT16 index = 0; index < OS_MAX_SEM; ++index) {
		mSems[index].setId(index);
		mIsFree[index] = TRUE;
		mFreeList.push_back(&mSems[index]);
	}
}

OSSem * OSSemManager::getFreeSem(UINT8 & err) {
	if (mFreeList.empty()) {
		err = ERR_NO_FREE_SEM;
		return NULL;
	}
	OSSem *free = mFreeList.front();
	mIsFree[free->getId()] = FALSE;
	mFreeList.pop_front();
	err = ERR_NO_ERR;
	return free;
}

void OSSemManager::returnSem(UINT16 id) {
	if (mIsFree[id]) {
		return;
	}
	mFreeList.push_back(&mSems[id]);
	mIsFree[id] = TRUE;
}

OSSem * OSSemManager::getSem(UINT16 id) {
	if (id >= OS_MAX_SEM) {
		return NULL;
	}
	if (mIsFree[id]) {
		
	}
	return &mSems[id];
}

OSSemManager * OSSemManager::getInstance() {
	if (gInstance == NULL) {
		gInstance = new OSSemManager();
	}
	return gInstance;
}

void OSSemManager::release() {
	delete gInstance;
	gInstance = NULL;
}
