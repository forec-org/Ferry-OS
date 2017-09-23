#pragma once

//----操作数类型---------------------------------------------------------------------

#define OP_TYPE_nullptr             -1          // 空数据
#define OP_TYPE_INT					0			//整形字面常量值
#define OP_TYPE_PTR					1			//整形指针
#define OP_TYPE_STRING_REF			2			//字符串索引
#define OP_TYPE_ABS_STACK_INDEX		3			//绝对堆栈索引
#define OP_TYPE_REL_STACK_INDEX		4			//相对堆栈索引
#define OP_TYPE_INSTR_INDEX			5			//指令索引
#define OP_TYPE_FUNC_INDEX			6			//函数索引
#define OP_TYPE_HOST_API_CALL_INDEX	7			//主应用程序API调用索引
#define OP_TYPE_REG					8			//寄存器

//----指令操作码---------------------------------------------------------------------

#define INSTR_MOV                   0			//赋值指令

#define INSTR_ADD                   1			//运算指令
#define INSTR_SUB                   2
#define INSTR_MUL                   3
#define INSTR_DIV                   4

#define INSTR_JMP                   5			//跳转指令
#define INSTR_JE                    6
#define INSTR_JNE                   7
#define INSTR_JG                    8
#define INSTR_JL                    9

#define INSTR_PUSH                  10			//堆栈指令
#define INSTR_POP                   11

#define INSTR_CALL                  12			//调用指令
#define INSTR_RET                   13
#define INSTR_IRET					14
#define INSTR_CALLCORE              15

#define INSTR_PAUSE                 16			//控制指令
#define INSTR_QUIT                  17

//----cpu配置----------------------------------------------------------------------

#define INSTR_LEN	32							//指令长度

#define CPU_RATE_M	5							//cpu频率

//----结构体-----------------------------------------------------------------------

//操作数
typedef struct _op {			
	int iType;									//操作数类型
	union {
		int iIntLiteral;						//整形字面量
		float fFloatLiteral;					//浮点型字面量
		int iStringTableIndex;					//字符串表索引
		int iStackIndex;						//堆栈索引
		int iInstrIndex;						//指令索引
		int iFuncIndex;							//函数索引
		int iHostAPICallIndex;					//主应用程序API调用索引
		int iReg;								//寄存器码
	};
	int iOffsetIndex;							//偏移量索引，仅在数据类型是堆栈索引时使用，存储堆栈偏移
}Op;

//指令
typedef struct _instr {
	unsigned short mOpCode;						//指令操作码
	unsigned char mOpNum;						//操作数数量
	Op mOpList[4];								//指令操作数
}Instr;

