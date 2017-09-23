#pragma once

#include "OS.h"

class OSSem;

//---------------------------------------------------------------------------------------
//
//	���̿��ƿ�
//
//		�ڴ���ջ�Ľṹ���£�						----------
//									mStackBase->				�͵�ַ
//													----------
//														.
//														.
//														.
//													----------
//									mStackTop->		
//													----------
//														.
//														.
//														.
//													----------
//									mStackBottom->				�ߵ�ַ
//													----------
//
//---------------------------------------------------------------------------------------
class OSPcb {
private:
	UINT32		mStackBottom;					//��ջ�ײ���ַ
	UINT32		mStackTop;						//ջ��
	UINT32		mStackBase;						//ջ��ַ

	STK_SIZE	mStackSize;						//��ջ�δ�С���ֽڣ�
	STK_SIZE	mStackUsed;						//��ǰʹ�õĶ�ջ�ռ�����

	UINT16		mPid;							//����id
		
	UINT16		mDelay;							//�������ʱ��¼��ʱʱ�䣨��������

	BOOLEAN		mIsTimeOut;						//�жϵȴ��ź����Ƿ�ʱ

	UINT8		mState;							//����״̬

	UINT8		mPrio;							//�������ȼ�

	BOOLEAN		mToDel;							//��־�����豻ɾ��

	UINT32		mSwitchCtr;						//��¼���񱻵���ִ�еĴ���
	UINT32		mTotalCycles;					//�������е���������

	OSSem		*mSemPtr;						//�ȴ����ź���ָ��

public:
	OSPcb();									//����ʱ����pid
	~OSPcb();

	void		init(UINT32 stackBase, UINT32 stackSize, UINT8 prio, UINT8 state = PROC_STATE_READY);

	void		clear();						//���������ƿ�

	void		setStackTop(UINT32 stackTop);
	void		setStackBase(UINT32 stackBase);
	UINT32		getStackTop();
	UINT32		getStackBase();

	void		setPid(UINT16 pid);				//��������pid			
	UINT16		getPid();						//��ȡ����pid

	UINT16		getDelay();						//�����ӳ�ʱ��
	void		delay(UINT16 delay);			//�����ӳ�
	void		decDelay();						//�ӳ�ʱ��ݼ�
	BOOLEAN		isTimeOut();					//�ж��Ƿ�ʱ
	void		setTimeOut(BOOLEAN isTimeOut);	//��־���̵ȴ���ʱ

	void		setState(UINT8 state);			//���ý���״̬
	UINT8		getState();						//��ȡ����״̬
	BOOLEAN		isWaitingEvent();				//�ж��Ƿ����ڵȴ��¼�����
	BOOLEAN		isSuspend();					//�ж��Ƿ����

	void		setPrio(UINT8 prio);			//�����������ȼ�
	UINT8		getPrio();						//��ȡ�������ȼ�

	void		setDel();						//��־�����豻ɾ��

	void		incSwitchCtr();					//����������ȴ���
	UINT32		getSwitchCtr();					//��ȡ������ȴ���

	void		incTotalCycles();				//��������ִ��������
	UINT32		getTotalCycles();				//��ȡ����ִ��������

	BOOLEAN		isNew();						//�ж��Ƿ��ǵ�һ�ν���CPU

	void		setSem(OSSem *psem);			//���õȴ����ź���
	OSSem		*getSem();						//��ȡ�ȴ����ź���
	void		removeFromSem();				//��������ӵȴ����ź���������ɾ��
};

