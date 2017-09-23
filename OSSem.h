#pragma once

#include "OS.h"
#include <list>

class OSPcb;

class OSSem {
private:
	UINT16				mId;							//ÿ���ź�����һ����ʶ
	UINT16				mCtr;							//�ź���������
	UINT16				mLargestCtr;					//�ź������������ֵ

	CHAR				mName[SEM_NAME_SIZE];			//�ź�����

	std::list<OSPcb*>	mWaitingList;					//�ȴ��ź����Ľ���

public:
	OSSem();
	~OSSem();

	void				init(UINT16 initCtr, 
							 UINT16 largestCtr, 
							 const char *name = NULL, 
							 UINT16 length = 0);

	bool				hasPend();

	BOOLEAN				wait(OSPcb *pcb,				
							 UINT16 delay);

	OSPcb				*signal();

	void				setId(UINT16 id);

	UINT16				getId();	

	void				setCtr(UINT16 ctr);

	UINT16				getCtr();

	void				setLargest(UINT16 largest);

	UINT16				getLargest();

	void				setName(const char *name, 
								UINT16 length);

	void				removeWaiting(UINT16 pid);		//�ӵȴ��������Ƴ���Ӧ�Ľ���
};

