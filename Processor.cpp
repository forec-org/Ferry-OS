#include "Processor.h"
#include "OSCore.h"
#include "OSPcb.h"
#include "Memory.h"
#include "IntController.h"
#include <thread>

Processor *Processor::gInstance = NULL;

void Processor::interprete() {
	if (mIR.mOpCode < 10) {
		CPUDebug(" ", 3);
		CPUDebug(mIR.mOpCode, 3);
	}
	else
		CPUDebug(mIR.mOpCode, 3);
	CPUDebug(" ", 3);
	CPUDebug(mnemonics[mIR.mOpCode], 3);
	CPUDebug(" ", 3);

	switch (mIR.mOpCode) {
	case INSTR_MOV:
	case INSTR_ADD:
	case INSTR_SUB:
	case INSTR_MUL:
	case INSTR_DIV:
		interpreteArithmetic();
		break;
	case INSTR_JMP:
	case INSTR_JE:
	case INSTR_JNE:
	case INSTR_JG:
	case INSTR_JL:
		interpreteJump();
		break;
	case INSTR_PUSH:
	case INSTR_POP:
		interpreteStack();
		break;
	case INSTR_CALL:
	case INSTR_CALLCORE:
	case INSTR_RET:
	case INSTR_IRET:
		interpreteCall();
		break;
	case INSTR_PAUSE:
	case INSTR_QUIT:
		interpreteCtrl();
		break;
	}
}

void Processor::interpreteArithmetic() {
	switch (mIR.mOpCode) {
	case INSTR_MOV:
	{
		int source = getValue(1);							//获取源操作数的值

		if (mIR.mOpList[0].iType == OP_TYPE_REG) {			//如果目的操作数是寄存器
			mReg[mIR.mOpList[0].iReg] = source;				//将源操作数的值存入相应寄存器
		}
		else {
			int destIndex = getOpStackIndex(0);				//获取目标操作数的堆栈地址
			setStackValue(destIndex, source);				//将源操作数的值存入堆栈
		}

		CPUDebug("source: ", 3);
		CPUDebugn(source, 3);
		break;
	}
	case INSTR_ADD:
	{
		int dest = getValue(0);								//获取目的操作数的值
		int source = getValue(1);							//获取源操作数的值
		int destStackIndex = getOpStackIndex(0);			//获取目的操作数的堆栈索引
		setStackValue(destStackIndex, dest + source);

		CPUDebug("source: ", 3);
		CPUDebug(source, 3);
		CPUDebug("dest: ", 3);
		CPUDebugn(dest, 3);

		break;
	}
	case INSTR_SUB:
	{
		int dest = getValue(0);
		int source = getValue(1);
		int destStackIndex = getOpStackIndex(0);
		setStackValue(destStackIndex, dest - source);

		CPUDebug("source: ", 3);
		CPUDebug(source, 3);
		CPUDebug("dest: ", 3);
		CPUDebugn(dest, 3);
		break;
	}
	case INSTR_MUL:
	{
		int dest = getValue(0);
		int source = getValue(1);
		int destStackIndex = getOpStackIndex(0);
		setStackValue(destStackIndex, dest * source);

		CPUDebug("source: ", 3);
		CPUDebug(source, 3);
		CPUDebug("dest: ", 3);
		CPUDebugn(dest, 3);
		break;
	}
	case INSTR_DIV:
	{
		int dest = getValue(0);
		int source = getValue(1);
		int destStackIndex = getOpStackIndex(0);
		setStackValue(destStackIndex, dest / source);

		CPUDebug("source: ", 3);
		CPUDebug(source, 3);
		CPUDebug("dest: ", 3);
		CPUDebugn(dest, 3);
		break;
	}
	default:
		break;
	}
}

void Processor::interpreteJump() {
	int op1;
	int op2;
	switch (mIR.mOpCode) {
	case INSTR_JMP:
		setPC(getValue(0));
		break;

	case INSTR_JE:
		op1 = getValue(0);
		op2 = getValue(1);

		if (op1 == op2) {
			setPC(getValue(2));
		}

		CPUDebug("op1: ", 3);
		CPUDebug(op1, 3);
		CPUDebug("op2: ", 3);
		CPUDebugn(op2, 3);

		break;

	case INSTR_JNE:
		op1 = getValue(0);
		op2 = getValue(1);

		if (op1 != op2) {
			setPC(getValue(2));
		}

		CPUDebug("op1: ", 3);
		CPUDebug(op1, 3);
		CPUDebug("op2: ", 3);
		CPUDebugn(op2, 3);

		break;

	case INSTR_JG:
		op1 = getValue(0);
		op2 = getValue(1);

		if (op1 > op2) {
			setPC(getValue(2));
		}

		CPUDebug("op1: ", 3);
		CPUDebug(op1, 3);
		CPUDebug("op2: ", 3);
		CPUDebugn(op2, 3);

		break;

	case INSTR_JL:
		op1 = getValue(0);
		op2 = getValue(1);

		if (op1 < op2) {
			setPC(getValue(2));
		}

		CPUDebug("op1: ", 3);
		CPUDebug(op1, 3);
		CPUDebug("op2: ", 3);
		CPUDebugn(op2, 3);

		break;
	default:
		break;
	}
}

void Processor::interpreteStack() {
	switch (mIR.mOpCode) {
	case INSTR_PUSH:
	{
		int value = getValue(0);
		push(value);
		break;

		CPUDebug("value: ", 3);
		CPUDebugn(value, 3);
	}
	case INSTR_POP:
	{
		int value;
		pop(value);
		if (mIR.mOpList[0].iType == OP_TYPE_REG) {
			mReg[mIR.mOpList[0].iReg] = value;
		}
		else {
			int stackIndex = getOpStackIndex(0);
			setStackValue(stackIndex, value);
		}

		CPUDebug("value: ", 3);
		CPUDebugn(value, 3);

		break;
	}

	default:
		break;
	}
}

void Processor::interpreteCall() {
	switch (mIR.mOpCode) {
	case INSTR_CALLCORE:
	{
		int strIndex = getValue(0);				//获取api函数名在api表中的下标
		int apiNameLen = 0;						//记录api函数名的长度
		char apiName[256];						//保存api函数名

		strIndex += mAS;

		apiNameLen = mMemory->readWord(strIndex);

		mMemory->readBlock((BYTE*)apiName, strIndex + sizeof(int), apiNameLen);
		apiName[apiNameLen] = '\0';

		CPUDebug("api: ", 3);
		CPUDebugn(apiName, 3);

		//调用操作系统api
		mCore->handleApiCall(apiName);
		break;
	}
	case INSTR_CALL:
	{
		int funcIndex = getValue(0);
		int funcLocalSize;
		int funcEntry;

		funcEntry = mMemory->readWord(mFS + funcIndex * 8);
		funcLocalSize = mMemory->readWord(mFS + funcIndex * 8 + 4);

		mBP = mSP - 3;								//记录函数帧底部位置
		push(mPC);									//保存返回后的指令地址
		pushFrame(funcLocalSize);					//压入堆栈帧
		push(funcLocalSize);						//压入栈帧大小

		setPC(funcEntry);							//设置pc为函数入口地址
		break;
	}
	case INSTR_RET:
	{
		int funcLocalSize;							//函数堆栈帧大小

		pop(funcLocalSize);							//弹出函数帧大小
		popFrame(funcLocalSize);					//弹出栈帧
		pop(mPC);									//弹出返回地址

		int nextFuncLocalSize;						//该函数下边的函数帧大小
		nextFuncLocalSize = getStackValue(mSP + 4);	//获取下边函数的函数帧大小
		mBP = mSP + 4 + nextFuncLocalSize + 1;		//重新定位BP	
	}
	default:
		break;
	}
}

void Processor::interpreteCtrl() {
	switch (mIR.mOpCode) {
	case INSTR_PAUSE:
		mCore->processDelay(getValue(0));
		break;
	case INSTR_QUIT:
		mCore->processQuit();
		break;
	}
}

int Processor::getOpStackIndex(unsigned int opIndex) {
	if (mIR.mOpList[opIndex].iType == OP_TYPE_ABS_STACK_INDEX) {					//绝对堆栈索引
		return mIR.mOpList[opIndex].iStackIndex;									//直接返回其堆栈索引
	}
	if (mIR.mOpList[opIndex].iType == OP_TYPE_REL_STACK_INDEX) {					//相对堆栈索引
		int iBaseIndex = mIR.mOpList[opIndex].iStackIndex;
		int iOffsetIndex = mIR.mOpList[opIndex].iOffsetIndex;						//获取变量的堆栈索引

		int stackValue = getStackValue(iOffsetIndex);							//获取变量数值

		return iBaseIndex + stackValue;											//计算出堆栈索引
	}
	return 0;
}

int Processor::getValue(unsigned int opIndex) {
	Op op = mIR.mOpList[opIndex];
	switch (op.iType) {
	case OP_TYPE_INT:								//数值常量
		return op.iIntLiteral;
	case OP_TYPE_PTR:								//指针变量
		return 0;
	case OP_TYPE_STRING_REF:						//字符串引用
		return op.iStringTableIndex;
	case OP_TYPE_ABS_STACK_INDEX:					//绝对堆栈索引
		return getStackValue(op.iStackIndex);
	case OP_TYPE_REL_STACK_INDEX:					//相对堆栈索引
		return getStackValue(getOpStackIndex(opIndex));
	case OP_TYPE_INSTR_INDEX:						//指令索引
		return op.iInstrIndex;
	case OP_TYPE_FUNC_INDEX:						//函数索引
		return op.iFuncIndex;
	case OP_TYPE_HOST_API_CALL_INDEX:				//内核api调用索引
		return op.iHostAPICallIndex;
	case OP_TYPE_REG:								//寄存器
		return mReg[op.iReg];
	default:
		return 0;
	}
}

int Processor::getStackValue(int stackIndex) {
	int value = 0;
	if (stackIndex >= 0) {				//全局变量
		value = mMemory->readWord(mSS + stackIndex);
	}
	else if (stackIndex < 0) {			//局部变量
		value = mMemory->readWord(mSS + (mBP + stackIndex)); 
	}
	return value;
}

void Processor::setStackValue(int stackIndex, int value) {
	if (stackIndex >= 0) {				//全局变量
		mMemory->writeWord(mSS + stackIndex, value);
	}
	else if (stackIndex < 0) {			//局部变量
		mMemory->writeWord(mSS + (mBP + stackIndex), value);
	}
}

void Processor::handleIrq() {
	if (getFlag(0) == true) {						//中断被屏蔽
		return;
	}
	if (mInt->hasIrq()) {							//有中断请求
		unsigned char intNum;						//中断类型号

		mInt->respondInt(intNum);					//响应中断并获取中断类型号

		CPUDebug("handle irq, int num: ", 3);
		CPUDebugn(intNum, 3);

		mCore->callIntService(intNum);				//调用对应中断服务
	}
}

void Processor::pushFrame(int size) {
	mSP -= size;
}

void Processor::popFrame(int size) {
	mSP += size;
}

Processor::Processor() {
}

void Processor::fetchIstr() {
	unsigned char IR[32];					//当前指令寄存器
	unsigned short count = 0;

	mMemory->readBlock(IR, mCS + mPC, INSTR_LEN);

	memcpy((void*)&(mIR.mOpCode), (void*)IR, sizeof(short));

	mIR.mOpNum = IR[sizeof(short)];

	count = 3;
	for (unsigned short index = 0; index < mIR.mOpNum; ++index) {
		mIR.mOpList[index].iType = IR[count];
		count += 1;
		switch (mIR.mOpList[index].iType) {
		case OP_TYPE_INT:
			memcpy((void*)&(mIR.mOpList[index].iIntLiteral), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_PTR:
			count += sizeof(int);
			break;
		case OP_TYPE_STRING_REF:
			memcpy((void*)&(mIR.mOpList[index].iStringTableIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_ABS_STACK_INDEX:
			memcpy((void*)&(mIR.mOpList[index].iStackIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_REL_STACK_INDEX:
			memcpy((void*)&(mIR.mOpList[index].iStackIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			memcpy((void*)&(mIR.mOpList[index].iOffsetIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_INSTR_INDEX:
			memcpy((void*)&(mIR.mOpList[index].iInstrIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_FUNC_INDEX:
			memcpy((void*)&(mIR.mOpList[index].iFuncIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_HOST_API_CALL_INDEX:
			memcpy((void*)&(mIR.mOpList[index].iHostAPICallIndex), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		case OP_TYPE_REG:
			memcpy((void*)&(mIR.mOpList[index].iReg), (void*)&(IR[count]), sizeof(int));
			count += sizeof(int);
			break;
		default:
			break;
		}
	}

	incPC();
}


Processor::~Processor() {
}

void Processor::init() {
	reset();
	
	mCore	= OSCore::getInstance();
	mMemory = Memory::getInstance();
	mInt	= IntController::getInstance();
}

void Processor::reset() {
	mFlag		&= 0;
	mPC			&= 0;
	mSS			&= 0;
	mSP			&= 0;

	mCS			&= 0;
	mDS			&= 0;
	mFS			&= 0;
	mAS			&= 0;

	mBP			&= 0;

	mReg[0]		&= 0;
	mReg[1]		&= 0;
	mReg[2]		&= 0;
	mReg[3]		&= 0;
}

void Processor::exec() {
	while (1) {
		fetchIstr();					//取指

		interprete();					//指令译码并执行

		handleIrq();					//处理中断

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / CPU_RATE_M));
	}
}

void Processor::setIM() {
	mFlag |= 0x00000001u;
}

void Processor::clearIM() {
	mFlag &= 0xfffffffeu;
}

void Processor::incPC() {
	mPC += INSTR_LEN;
}

void Processor::setPC(unsigned int instrCount) {
	mPC = instrCount * INSTR_LEN;
}

void Processor::setSS(REGISTER ss) {
	mSS = ss;
}

void Processor::setSP(REGISTER sp) {
	mSP = sp;
}

void Processor::setCS(REGISTER cs) {
	mCS = cs;
}

void Processor::setDS(REGISTER ds) {
	mDS = ds;
}

void Processor::setFS(REGISTER fs) {
	mFS = fs;
}

void Processor::setAS(REGISTER as) {
	mAS = as;
}

void Processor::setBP(REGISTER bp) {
	mBP = bp;
}

void Processor::setFlag(int index) {
	mFlag |= (1u << index);
}

void Processor::clearFlag(int index) {
	mFlag &= ~(1u << index);
}

REGISTER Processor::getPC() {
	return mPC;
}

REGISTER Processor::getSS() {
	return mSS;
}

REGISTER Processor::getSP() {
	return mSP;
}

REGISTER Processor::getCS() {
	return mCS;
}

REGISTER Processor::getDS() {
	return mDS;
}

REGISTER Processor::getFS() {
	return mFS;
}

REGISTER Processor::getAS() {
	return mAS;
}

REGISTER Processor::getBP() {
	return mBP;
}

bool Processor::getFlag(int index) {
	return (mFlag >> index) & 1u;
}

void Processor::setReg(unsigned short index, REGISTER num) {
	mReg[index] = num;
}

REGISTER Processor::getReg(unsigned short index) {
	return mReg[index];
}


void Processor::saveContext() {
	OSPcb *curr = mCore->getPcbCurr();
	CPUDebug("cpu save proc content, pid:", 3);
	CPUDebugn(curr->getPid(), 3);

	for (unsigned short index = 0; index < 4; ++index) {				//保存寄存器值
		push(mReg[index]);
	}
	push(mCS);															//保存代码段基址
	push(mDS);															//保存数据段基址
	push(mFS);															//保存函数表基址
	push(mAS);															//保存系统API调用表基址
	push(mBP);															//保存当前函数帧地址

	push(mPC);															//程序计数器
	curr->setStackTop(mSP);												//保存堆栈指针
	curr->setStackBase(mSS);									
}

void Processor::reloadContext() {										
	OSPcb *curr = mCore->getPcbCurr();

	CPUDebug("load sp and ss.", 3);
	mSS = curr->getStackBase();											//获取栈基址
	mSP = curr->getStackTop();											//获取栈顶地址
	CPUDebug("ss:", 3);
	CPUDebug(mSS, 3);
	CPUDebug("sp:", 3);
	CPUDebugn(mSP, 3);

	pop(mPC);
	pop(mBP);
	pop(mAS);
	pop(mFS);
	pop(mDS);
	pop(mCS);
	for (short index = 3; index >= 0; index--) {
		pop(mReg[index]);
	}
}

void Processor::push(int num) {
	mMemory->writeWord(mSS + mSP - 3, num);
	mSP -= 4;
}

void Processor::pop(int & num) {
	num = mMemory->readWord(mSS + mSP + 1);
	mSP += 4;
}

Processor * Processor::getInstance() {
	if (gInstance == NULL) {
		gInstance = new Processor();
	}
	return gInstance;
}

void Processor::release() {
	delete gInstance;
	gInstance = NULL;
}

void Processor::CPUDebug(const char * tip, short level) {
#if CPU_DEBUG_ENABLE
	if (level <= CPU_DEBUG_LEVEL) {
		printf(tip);
	}
#endif // DEBUG_ENABLE
}

void Processor::CPUDebugn(const char * tip, short level) {
#if CPU_DEBUG_ENABLE
	if (level <= CPU_DEBUG_LEVEL) {
		printf(tip);
		printf("\n");
	}
#endif // DEBUG_ENABLE
}

void Processor::CPUDebug(int tip, short level) {
#if CPU_DEBUG_ENABLE
	if (level <= CPU_DEBUG_LEVEL) {
		printf("%d", tip);
	}
#endif // DEBUG_ENABLE
}

void Processor::CPUDebugn(int tip, short level) {
#if CPU_DEBUG_ENABLE
	if (level <= CPU_DEBUG_LEVEL) {
		printf("%d\n", tip);
	}
#endif // DEBUG_ENABLE
}
