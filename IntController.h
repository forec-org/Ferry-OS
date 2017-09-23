#pragma once

//	�жϿ����������Թ���8���ⲿ�ж�Դ
class IntController {
	static IntController *gInstance;
	static unsigned char highestPrioMap[256];		

private:
	bool			mIrq;								//��IRR�Ĵ�����Ϊ0ʱ��1

	unsigned char	mIrr;								//�ж�����Ĵ�����Ϊ1��ʾ���ж��������ȼ�Ϊ0��ߣ�7��ͣ�����λΪ0
	
	unsigned char	mIsr;								//���ڷ���Ĵ�����������ڱ�������жϡ�ĳλΪ1��ʾIRi�ж����ڷ���
														//�ж�Ƕ��ʱ�ж������ͬʱ����1��Ŀǰ�ں�ֻ֧�ֵ����жϣ�

	unsigned char	mIntNum;							//���ȼ���ߵ��ж����ͺ�

	IntController();

	bool prioJudge();									//����Ȩ�о�

public:
	
	~IntController();

	void			init();

	void			setIntReq(unsigned char index);		//��λ�ж��������������

	void			clearIntReq(unsigned char index);	//�����ж�����	

	void			setIsr(unsigned short index);		//��λ���ڷ���Ĵ�������ʾ��Ӧ�ж����ڱ�����

	void			clearIsr(unsigned char index);		//�������ڷ���Ĵ�������ʾ�жϷ������

	bool			hasIrq();							//cpu���ã��ж��Ƿ����ж�

	void			respondInt(unsigned char &intNum);	//cpu����,��Ӧ�жϣ���ȡ�ж����ͺţ����ж�����Ĵ�����Ӧλ���㣬��λ���ڷ���Ĵ�����Ӧλ

	//void endInt();									//cpu���ã������ж�

	static IntController *getInstance();
	static void release();
};







//��һ��INTA��8259A���ȼ���ߵ���������Ӧ��ISR�е�λ��1������ӦIRR�е�λ��֮��λ
//�ڶ���INTA�����߽�����8259A����ǰ�ж������Ӧ���ж����ͺ��͵�����������

//�жϽ���
//��ͨEOI
//�жϷ����������8259A ���жϽ������EOI������ISR�Ķ�Ӧbit��0���Ա�ʶ�жϽ�����
//�Զ�EOI
//�жϳ���������EOI����ڵڶ���INTA�����źŵĺ��أ���ISR�Ķ�Ӧbit��0��
