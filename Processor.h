#pragma once

#include "CpuConfig.h"

class OSCore;
class Memory;
class IntController;

typedef int REGISTER;

//	模拟CPU
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
	REGISTER	mFlag;								//标志寄存器
													//0位-IM：标志是否屏蔽中断,为1屏蔽，为0允许
													//其余暂未设定
	
	REGISTER	mPC;								//程序计数器

	REGISTER	mSS;								//堆栈段基址寄存器
	REGISTER	mSP;								//堆栈栈顶寄存器

	REGISTER	mCS;								//代码段基址寄存器
	REGISTER	mDS;								//数据段基址寄存器（用来存字符串表首地址）
	REGISTER	mFS;								//函数表基址寄存器
	REGISTER	mAS;								//系统API调用表基址寄存器

	REGISTER	mBP;								//函数帧寄存器，用于记录当前函数帧的位置

	Instr		mIR;								//当前指令寄存器

	REGISTER	mReg[4];							//通用寄存器，0号寄存器：函数调用返回值
													//			  1号寄存器：系统调用返回值

	OSCore		*mCore;								//操作系统引用
	Memory		*mMemory;							//内存引用
	IntController *mInt;							//中断控制器引用

private://method
	Processor();

	void		fetchIstr();						//取指令

	void		interprete();						//解释指令
	void		interpreteArithmetic();				//解释运算指令(双操作数）
	void		interpreteJump();					//解释跳转指令
	void		interpreteStack();					//解释堆栈指令
	void		interpreteCall();					//解释调用指令
	void		interpreteCtrl();					//解释控制类指令

	void		handleIrq();						//每次执行时判断是否有中断请求要处理

	int			getOpStackIndex(unsigned int opIndex);			
													//获取操作数在栈中的索引，index为操作数下标

	int			getValue(unsigned int opIndex);					
													//获取操作数实际值，index为操作数下标

	int			getStackValue(int stackIndex);					
													//根据堆栈偏移量获取栈中的值
													//	偏移量为正说明时全局变量，从堆栈基地址寻址
													//	为负说明是函数局部变量，从函数帧寻址

	void		setStackValue(int stackIndex, int value);		
													//将运算结果保存到变量中

	void		pushFrame(int size);				//压入函数栈帧，单位为字节
	void		popFrame(int size);					//弹出函数栈帧，单位为字节


public://method
	
	~Processor();

	void		init();								//初始化CPU

	void		reset();							//重置CPU

	void		exec();								//循环执行指令

	void		setIM();							//将中断屏蔽位置位

	void		clearIM();							//将中断屏蔽位清零

	void		incPC();							//程序计数器递增
	void		setPC(unsigned int instrCount);		//设置程序计数器，传入的是指令序号
	void		setSS(REGISTER ss);					//设置栈段址寄存器	
	void		setSP(REGISTER sp);					//设置栈顶寄存器
	void		setCS(REGISTER cs);					//设置代码段基地址寄存器
	void		setDS(REGISTER ds);					//设置数据段基地址寄存器
	void		setFS(REGISTER fs);					//设置函数表基地址寄存器
	void		setAS(REGISTER as);					//设置系统调用api表基地址寄存器
	void		setBP(REGISTER bp);					//设置当前函数帧地址寄存器
	void		setFlag(int index);
	void		clearFlag(int index);

	REGISTER	getPC();							//获取PC
	REGISTER	getSS();							//获取堆栈段址
	REGISTER	getSP();							//获取堆栈栈顶
	REGISTER	getCS();							//获取代码段基地址寄存器
	REGISTER	getDS();							//获取数据段基地址寄存器
	REGISTER	getFS();							//获取函数表基地址寄存器
	REGISTER	getAS();							//获取系统调用api表基地址寄存器
	REGISTER	getBP();							//获取当前函数帧地址寄存器
	bool		getFlag(int index);

	void		setReg(unsigned short index, REGISTER num);
													//设置通用寄存器
	REGISTER	getReg(unsigned short index);		//获取通用寄存器

	void		saveContext();						//保存进程信息至程序堆栈中

	void		reloadContext();					//从程序堆栈中载入进程信息

	void		push(int num);						//堆栈操作

	void		pop(int &num);						//弹栈操作

	static Processor *getInstance();
	static void release();


#define CPU_DEBUG_ENABLE 0
#define CPU_DEBUG_LEVEL 3
	void CPUDebug(const char *tip, short level);
	void CPUDebugn(const char *tip, short level);
	void CPUDebug(int tip, short level);
	void CPUDebugn(int tip, short level);
};

