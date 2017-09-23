#pragma once

#include "OS.h"
#include "SystemConfig.h"
#include "OSSem.h"
#include <list>

typedef std::list<OSSem*> OSSemList;

class OSSemManager {
	static OSSemManager *gInstance;

private:
	OSSem				mSems[OS_MAX_SEM];				//静态分配信号量
	BOOLEAN				mIsFree[OS_MAX_SEM];			//判断信号量是否被创建
	OSSemList			mFreeList;						//空闲列表

	OSSemManager();

public:
	~OSSemManager();

	void				init();

	OSSem				*getFreeSem(UINT8 &err);		//获取空闲信号量

	void				returnSem(UINT16 id);			//返还信号量

	OSSem				*getSem(UINT16 id);				//获取已创建的信号量

	static OSSemManager *getInstance();		
	static void release();
};

