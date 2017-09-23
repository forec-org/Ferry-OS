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

	OSPcb *free = mFreeList.front();			//��ȡ����ͷ�Ŀ���PCB�����Ӷ�����ɾ��
	mFreeList.pop_front();
	mCreated[free->getPid()] = TRUE;			//��־��PCB�ѱ�ռ��
	mProcCtr++;									//�������������
	err = ERR_NO_ERR;
	return free;
}

void OSPcbManager::returnPCB(OSPcb *pcb) {
	pcb->clear();								//���pcb����
	mFreeList.push_back(pcb);					//��������б�
	mCreated[pcb->getPid()] = FALSE;			//���δ��ռ��
	mProcCtr--;									//�������ݼ�
}

OSPcb * OSPcbManager::getPCB(UINT16 pid) {
	if (isCreated(pid)) {
		return &mPcbs[pid];
	}
	return NULL;
}
