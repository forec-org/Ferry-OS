#pragma once

#include "OS.h"
#include <string>
#include <unordered_map>

class OSPcb;
class OSPcbManager;
class OSScheduler;
class Processor;
class OSSemManager;
struct OSProcFile;
class Memory;

class OSCore {
	static OSCore *gInstance;					

	typedef void (OSCore::*API_FUNC)(void);					//api����ָ��
	typedef std::unordered_map<std::string, API_FUNC> APITABLE;

private:
	BOOLEAN			mIsRunning;								//�ں��Ƿ�����
	UINT8			mIntNesting;							//�ж�Ƕ�ײ�����Ŀǰ֧�ֵ����ж�

	OSPcbManager	*mPcbManager;							//���̿��ƿ����ģ��
	OSScheduler		*mScheduler;							//����ģ��
	OSSemManager	*mSemManager;							//�ź�������ģ��

	APITABLE		mApiTable;								//���û�������õ�api��

	UINT32			mTime;									//ϵͳʱ�䣨����ֵ��
	UINT16			mFreeTime;								//1S�п��е�CPU�������������ڼ���CPU������
	F32				mCPUUsage;								//cpu������

	Processor		*mCPU;									//CPUģ��
	Memory			*mMem;									//�ڴ�ģ��

private:
	void			initIdle();								//��ʼ����ת����
	
	void			initTestProc();							//��ʼ�����Գ���

	void			initApiTable();							//��ʼ��api��					

	UINT8			loadExecFile(const char *path,			//װ�ؿ�ִ���ļ�
								 OSPcb *pcb, 
								 UINT32 base);				
															

	OSCore();

public:													
	BOOLEAN			handleApiCall(const std::string &funcName);		
															//������̵�ϵͳ����

	void			incFreeTime();							//����CPU����������
	void			clearFreeTime();						//��տ���CPU������

	void			callIntService(UINT8 intNum);			//�����жϷ������

	//���û�������õ�api���ö�ջ���Σ�����ֵ�浽1�żĴ���

	void			api_print();							//����Ϊ�ַ������ַ������е�����
	void			api_process_create();					//����Ϊ����·���ַ����ı�������
	void			api_process_delete();					//����Ϊ����id
	void			api_sem_accept();						//����Ϊ�ź���id
	void			api_sem_pend();							//����Ϊ�ȴ��ź���id�͵ȴ�ʱ��
	void			api_sem_post();							//����Ϊ�ź���id
	void			api_sem_delete();						//����Ϊ�ź���id
	void			api_sem_create();						//����Ϊ�ź�����ʼ����ֵ��������ֵ

public:											
	
	~OSCore();
	
	void			init();									//��ʼ���ں�

	void			start();								//��ʼ����ϵͳ�Ķ��������

	void			timeTick();								//ʱ���жϵ��ã�ͳ��ϵͳʱ�䣬
															//�ж��Ƿ�Ӧ�ý�����ת����

	OSPcb			*getPcbCurr();							//��ȡ��ǰpcb

	UINT16			getFreeTime();							//��ȡ����CPU������

	void			print(const std::string &text);			//��ӡ

	INT8			processCreate(const char *path,			//���̴���
								  UINT8 &err, 
								  UINT8 prio = 0);		

	UINT8			processDelete(UINT16 pid);				//ɾ������
	UINT8			processDelay(UINT16 delay);				//���������ʱ��
	UINT8			processQuit();							//�����˳�	

	BOOLEAN			semAccept(UINT16 id);					//�������صȴ��ź���
	INT16			semCreate(UINT16 ctr,					//�����ź����������ź���id
							  UINT16 lgst, 
							  UINT8 &err);
															
	UINT8			semDelete(UINT16 id);					//�ź���ɾ��
	UINT8			semPend(UINT16 id, UINT16 delay);		//�ȴ��ź�����delayΪ0��ʾ���õȴ�
	UINT8			semPost(UINT16 id);						//�ͷ��ź���

	void			intEnter();								//��֪����ϵͳ�����жϷ������,��ʱ�����ò���
	void			intExit();								//�˳��жϴ���
	void			enterCritical();						//����ϵͳ�ٽ�������Ϊ�ں˲���cpu��ִ�У������ʱ�ò���
	void			exitCritical();							//�뿪ϵͳ�ٽ�����ͬ��

	static OSCore	*getInstance();		
	static void		release();
};

