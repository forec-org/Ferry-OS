#pragma once

#include "OS.h"
#include "SystemConfig.h"
#include "OSSem.h"
#include <list>

typedef std::list<OSSem*> OSSemList;

class OSSemManager {
	static OSSemManager *gInstance;

private:
	OSSem				mSems[OS_MAX_SEM];				//��̬�����ź���
	BOOLEAN				mIsFree[OS_MAX_SEM];			//�ж��ź����Ƿ񱻴���
	OSSemList			mFreeList;						//�����б�

	OSSemManager();

public:
	~OSSemManager();

	void				init();

	OSSem				*getFreeSem(UINT8 &err);		//��ȡ�����ź���

	void				returnSem(UINT16 id);			//�����ź���

	OSSem				*getSem(UINT16 id);				//��ȡ�Ѵ������ź���

	static OSSemManager *getInstance();		
	static void release();
};

