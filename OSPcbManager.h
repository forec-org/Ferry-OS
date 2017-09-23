#pragma once

#include "OS.h"
#include "SystemConfig.h"
#include <list>
#include "OSPcb.h"

typedef std::list<OSPcb*> OSPcbList;

class OSPcbManager {
	static OSPcbManager *gInstance;

private:
	OSPcbList			mFreeList;						//����PCB�б�
	BOOLEAN				mCreated[OS_MAX_PCB];			//PCB�������
	OSPcb				mPcbs[OS_MAX_PCB];				//PCB�б�
	UINT16				mProcCtr;						//�Ѵ����������

	OSPcbManager();
public:
	
	~OSPcbManager();

	void				init();							//��ʼ������PCB�б�

	OSPcb				*getFreePCB(UINT8 &err);		//��ȡ����pcb

	void				returnPCB(OSPcb *pcb);			//����ɾ���󷵻�PCB

	OSPcb				*getPCB(UINT16 pid);			//��ȡ�Ѵ��������PCB

	UINT16				getProcNum();					//��ȡ��ǰ������

	BOOLEAN				isCreated(UINT16 pid);			//�ж������Ƿ񱻴���

	static OSPcbManager *getInstance();					//��ȡʵ��

	static void			release();						//ɾ��ʵ��
};

