#pragma once

//	中断控制器，可以管理8个外部中断源
class IntController {
	static IntController *gInstance;
	static unsigned char highestPrioMap[256];		

private:
	bool			mIrq;								//当IRR寄存器不为0时置1

	unsigned char	mIrr;								//中断请求寄存器，为1表示有中断请求，优先级为0最高，7最低，最右位为0
	
	unsigned char	mIsr;								//正在服务寄存器，存放正在被服务的中断。某位为1表示IRi中断正在服务
														//中断嵌套时有多个比特同时被置1（目前内核只支持单级中断）

	unsigned char	mIntNum;							//优先级最高的中断类型号

	IntController();

	bool prioJudge();									//优先权判决

public:
	
	~IntController();

	void			init();

	void			setIntReq(unsigned char index);		//置位中断请求，由外设调用

	void			clearIntReq(unsigned char index);	//清零中断请求	

	void			setIsr(unsigned short index);		//置位正在服务寄存器，表示对应中断正在被服务

	void			clearIsr(unsigned char index);		//清零正在服务寄存器，表示中断服务完成

	bool			hasIrq();							//cpu调用，判断是否有中断

	void			respondInt(unsigned char &intNum);	//cpu调用,响应中断，获取中断类型号，将中断请求寄存器对应位清零，置位正在服务寄存器对应位

	//void endInt();									//cpu调用，结束中断

	static IntController *getInstance();
	static void release();
};







//第一个INTA，8259A优先级最高的请求所对应的ISR中的位置1，而相应IRR中的位随之复位
//第二个INTA，总线解锁，8259A将当前中断请求对应的中断类型号送到数据总线上

//中断结束
//普通EOI
//中断服务程序需向8259A 送中断结束命令（EOI），将ISR的对应bit清0，以标识中断结束。
//自动EOI
//中断程序无需送EOI命令。在第二个INTA脉冲信号的后沿，将ISR的对应bit清0。
