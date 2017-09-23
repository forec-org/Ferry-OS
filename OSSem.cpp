#include "OSSem.h"
#include "OSPcb.h"


OSSem::OSSem() {
}


OSSem::~OSSem() {
}

void OSSem::init(UINT16 initCtr, UINT16 largestCtr, const char * name, UINT16 length) {
	mCtr = initCtr;
	mLargestCtr = largestCtr;
	setName(name, length);
}

bool OSSem::hasPend() {
	return mWaitingList.size() != 0;
}

BOOLEAN OSSem::wait(OSPcb *pcb, UINT16 delay) {
	if (mCtr > 0) {									//���õȴ�
		mCtr--;
		return TRUE;
	}

	pcb->setState(PROC_STATE_WAITING);				//����pcb״̬�ͳ�ʱ��Ϣ
	pcb->setTimeOut(FALSE);
	pcb->delay(delay);								//���ó�ʱʱ��
	pcb->setSem(this);
	mWaitingList.push_back(pcb);					//����ȴ�����
	return FALSE;
}

OSPcb *OSSem::signal() {
	if (mWaitingList.empty()) {
		if (mCtr < mLargestCtr) {					//���û�еȴ����̣��ͰѼ���������
			mCtr++;
		}
		return NULL;
	}

	OSPcb *toReady = mWaitingList.front();			//��ȡ����������ͷ�Ľ���

													//���ý���״̬Ϊ����
	if (toReady->getState() == PROC_STATE_SWAPPED_WAITING) {
		toReady->setState(PROC_STATE_SWAPPED_READY);
	}
	else {
		toReady->setState(PROC_STATE_READY);
	}

	toReady->setTimeOut(FALSE);			
	toReady->delay(0);
	toReady->setSem(NULL);
	mWaitingList.pop_front();
	return toReady;
}

void OSSem::setId(UINT16 id) {
	mId = id;
}

UINT16 OSSem::getId() {
	return mId;
}

void OSSem::setCtr(UINT16 ctr) {
	if (ctr > mLargestCtr) {
		return;
	}
	mCtr = ctr;
}

UINT16 OSSem::getCtr() {
	return mCtr;
}

void OSSem::setLargest(UINT16 largest) {
	if (largest < mCtr) {
		return;
	}
	mLargestCtr = largest;
}

UINT16 OSSem::getLargest() {
	return mLargestCtr;
}

void OSSem::setName(const char * name, UINT16 length) {
	if (name == NULL) {
		memset(mName, 0, sizeof(mName));
		return;
	}
	if (length > SEM_NAME_SIZE) {
		length = SEM_NAME_SIZE - 1;
	}
	memcpy(mName, name, length);
	mName[length] = '\0';
}

void OSSem::removeWaiting(UINT16 pid) {
	for (auto iter = mWaitingList.begin(); iter != mWaitingList.end(); ++iter) {
		if ((*iter)->getPid() == pid) {
			mWaitingList.erase(iter);
			return;
		}
	}
}
