#pragma once

#include "OS.h"
#include <list>

//class OSPcbManager;
class OSPcb;

typedef std::list<OSPcb*> OSPcbList;

class OSScheduler {
	static OSScheduler *gInstance;
private:
	OSPcb			*mPcbCurr;							//��ǰ����ִ�еĽ���PCB
	OSPcbList		mWaitingList;						//�ȴ������б�
	OSPcbList		mReadyList;							//����̬�����б�
	
	OSPcb			*mNextToRun;						//��һ����Ҫִ�еĽ���

	OSPcb			*mIdlePcb;							//��ת����PCB

	BOOLEAN			getNextToRun();						//��ȡ��һ��Ҫ����CPUִ�еĽ���

	OSScheduler();
public:
					
	~OSScheduler();

	void			init();								//��ʼ��

	void			sched();							//���ȳ���

	OSPcb			*getCurrPcb();						//��ȡ��ǰ����ִ�еĽ��̿��ƿ�

	void			setIdlePcb(OSPcb *idlePcb);			//���ÿ�ת����

	void			caculateDelay();					//����ȴ�������ÿ������ĵȴ�ʱ��

	BOOLEAN			toSwapOutCurr();					//�жϵ�ǰ�����Ƿ�Ӧ�ñ�����

	void			addReadyPcb(OSPcb *pcb);			//�����̼����������						
	void			addWaitingPcb(OSPcb *pcb);			//�����̼���ȴ�����
	void			removeFromReadyList(UINT16 pid);	//�Ӿ����б���ɾ��ĳ��pcb
	void			removeFromWaitingList(UINT16 pid);	//�ӵȴ��б���ɾ��ĳ��pcb

	OSPcb*			removeRunning();					//�Ƴ��������еĽ���

	UINT16			getReadyNum();						//��ȡ������������
	UINT16			getWaitingNum();					//��ȡ�ȴ���������

	static OSScheduler *getInstance();

	static void release();
};

