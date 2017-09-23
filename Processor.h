#pragma once

#include "CpuConfig.h"

class OSCore;
class Memory;
class IntController;

typedef int REGISTER;

//	ģ��CPU
class Processor {
	static Processor *gInstance;

	char mnemonics[18][12] =
	{
		"Mov",
		"Add", "Sub", "Mul", "Div",
		"Jmp", "JE", "JNE", "JG", "JL",
		"Push", "Pop",
		"Call", "Ret", "Iret", "CallHost",
		"Pause", "Quit"
	};

private://member
	REGISTER	mFlag;								//��־�Ĵ���
													//0λ-IM����־�Ƿ������ж�,Ϊ1���Σ�Ϊ0����
													//������δ�趨
	
	REGISTER	mPC;								//���������

	REGISTER	mSS;								//��ջ�λ�ַ�Ĵ���
	REGISTER	mSP;								//��ջջ���Ĵ���

	REGISTER	mCS;								//����λ�ַ�Ĵ���
	REGISTER	mDS;								//���ݶλ�ַ�Ĵ������������ַ������׵�ַ��
	REGISTER	mFS;								//�������ַ�Ĵ���
	REGISTER	mAS;								//ϵͳAPI���ñ��ַ�Ĵ���

	REGISTER	mBP;								//����֡�Ĵ��������ڼ�¼��ǰ����֡��λ��

	Instr		mIR;								//��ǰָ��Ĵ���

	REGISTER	mReg[4];							//ͨ�üĴ�����0�żĴ������������÷���ֵ
													//			  1�żĴ�����ϵͳ���÷���ֵ

	OSCore		*mCore;								//����ϵͳ����
	Memory		*mMemory;							//�ڴ�����
	IntController *mInt;							//�жϿ���������

private://method
	Processor();

	void		fetchIstr();						//ȡָ��

	void		interprete();						//����ָ��
	void		interpreteArithmetic();				//��������ָ��(˫��������
	void		interpreteJump();					//������תָ��
	void		interpreteStack();					//���Ͷ�ջָ��
	void		interpreteCall();					//���͵���ָ��
	void		interpreteCtrl();					//���Ϳ�����ָ��

	void		handleIrq();						//ÿ��ִ��ʱ�ж��Ƿ����ж�����Ҫ����

	int			getOpStackIndex(unsigned int opIndex);			
													//��ȡ��������ջ�е�������indexΪ�������±�

	int			getValue(unsigned int opIndex);					
													//��ȡ������ʵ��ֵ��indexΪ�������±�

	int			getStackValue(int stackIndex);					
													//���ݶ�ջƫ������ȡջ�е�ֵ
													//	ƫ����Ϊ��˵��ʱȫ�ֱ������Ӷ�ջ����ַѰַ
													//	Ϊ��˵���Ǻ����ֲ��������Ӻ���֡Ѱַ

	void		setStackValue(int stackIndex, int value);		
													//�����������浽������

	void		pushFrame(int size);				//ѹ�뺯��ջ֡����λΪ�ֽ�
	void		popFrame(int size);					//��������ջ֡����λΪ�ֽ�


public://method
	
	~Processor();

	void		init();								//��ʼ��CPU

	void		reset();							//����CPU

	void		exec();								//ѭ��ִ��ָ��

	void		setIM();							//���ж�����λ��λ

	void		clearIM();							//���ж�����λ����

	void		incPC();							//�������������
	void		setPC(unsigned int instrCount);		//���ó�����������������ָ�����
	void		setSS(REGISTER ss);					//����ջ��ַ�Ĵ���	
	void		setSP(REGISTER sp);					//����ջ���Ĵ���
	void		setCS(REGISTER cs);					//���ô���λ���ַ�Ĵ���
	void		setDS(REGISTER ds);					//�������ݶλ���ַ�Ĵ���
	void		setFS(REGISTER fs);					//���ú��������ַ�Ĵ���
	void		setAS(REGISTER as);					//����ϵͳ����api�����ַ�Ĵ���
	void		setBP(REGISTER bp);					//���õ�ǰ����֡��ַ�Ĵ���
	void		setFlag(int index);
	void		clearFlag(int index);

	REGISTER	getPC();							//��ȡPC
	REGISTER	getSS();							//��ȡ��ջ��ַ
	REGISTER	getSP();							//��ȡ��ջջ��
	REGISTER	getCS();							//��ȡ����λ���ַ�Ĵ���
	REGISTER	getDS();							//��ȡ���ݶλ���ַ�Ĵ���
	REGISTER	getFS();							//��ȡ���������ַ�Ĵ���
	REGISTER	getAS();							//��ȡϵͳ����api�����ַ�Ĵ���
	REGISTER	getBP();							//��ȡ��ǰ����֡��ַ�Ĵ���
	bool		getFlag(int index);

	void		setReg(unsigned short index, REGISTER num);
													//����ͨ�üĴ���
	REGISTER	getReg(unsigned short index);		//��ȡͨ�üĴ���

	void		saveContext();						//���������Ϣ�������ջ��

	void		reloadContext();					//�ӳ����ջ�����������Ϣ

	void		push(int num);						//��ջ����

	void		pop(int &num);						//��ջ����

	static Processor *getInstance();
	static void release();


#define CPU_DEBUG_ENABLE 0
#define CPU_DEBUG_LEVEL 3
	void CPUDebug(const char *tip, short level);
	void CPUDebugn(const char *tip, short level);
	void CPUDebug(int tip, short level);
	void CPUDebugn(int tip, short level);
};

