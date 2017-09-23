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
		int source = getValue(1);							//��ȡԴ��������ֵ

		if (mIR.mOpList[0].iType == OP_TYPE_REG) {			//���Ŀ�Ĳ������ǼĴ���
			mReg[mIR.mOpList[0].iReg] = source;				//��Դ��������ֵ������Ӧ�Ĵ���
		}
		else {
			int destIndex = getOpStackIndex(0);				//��ȡĿ��������Ķ�ջ��ַ
			setStackValue(destIndex, source);				//��Դ��������ֵ�����ջ
		}

		CPUDebug("source: ", 3);
		CPUDebugn(source, 3);
		break;
	}
	case INSTR_ADD:
	{
		int dest = getValue(0);								//��ȡĿ�Ĳ�������ֵ
		int source = getValue(1);							//��ȡԴ��������ֵ
		int destStackIndex = getOpStackIndex(0);			//��ȡĿ�Ĳ������Ķ�ջ����
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
		int strIndex = getValue(0);				//��ȡapi��������api���е��±�
		int apiNameLen = 0;						//��¼api�������ĳ���
		char apiName[256];						//����api������

		strIndex += mAS;

		apiNameLen = mMemory->readWord(strIndex);

		mMemory->readBlock((BYTE*)apiName, strIndex + sizeof(int), apiNameLen);
		apiName[apiNameLen] = '\0';

		CPUDebug("api: ", 3);
		CPUDebugn(apiName, 3);

		//���ò���ϵͳapi
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

		mBP = mSP - 3;								//��¼����֡�ײ�λ��
		push(mPC);									//���淵�غ��ָ���ַ
		pushFrame(funcLocalSize);					//ѹ���ջ֡
		push(funcLocalSize);						//ѹ��ջ֡��С

		setPC(funcEntry);							//����pcΪ������ڵ�ַ
		break;
	}
	case INSTR_RET:
	{
		int funcLocalSize;							//������ջ֡��С

		pop(funcLocalSize);							//��������֡��С
		popFrame(funcLocalSize);					//����ջ֡
		pop(mPC);									//�������ص�ַ

		int nextFuncLocalSize;						//�ú����±ߵĺ���֡��С
		nextFuncLocalSize = getStackValue(mSP + 4);	//��ȡ�±ߺ����ĺ���֡��С
		mBP = mSP + 4 + nextFuncLocalSize + 1;		//���¶�λBP	
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
	if (mIR.mOpList[opIndex].iType == OP_TYPE_ABS_STACK_INDEX) {					//���Զ�ջ����
		return mIR.mOpList[opIndex].iStackIndex;									//ֱ�ӷ������ջ����
	}
	if (mIR.mOpList[opIndex].iType == OP_TYPE_REL_STACK_INDEX) {					//��Զ�ջ����
		int iBaseIndex = mIR.mOpList[opIndex].iStackIndex;
		int iOffsetIndex = mIR.mOpList[opIndex].iOffsetIndex;						//��ȡ�����Ķ�ջ����

		int stackValue = getStackValue(iOffsetIndex);							//��ȡ������ֵ

		return iBaseIndex + stackValue;											//�������ջ����
	}
	return 0;
}

int Processor::getValue(unsigned int opIndex) {
	Op op = mIR.mOpList[opIndex];
	switch (op.iType) {
	case OP_TYPE_INT:								//��ֵ����
		return op.iIntLiteral;
	case OP_TYPE_PTR:								//ָ�����
		return 0;
	case OP_TYPE_STRING_REF:						//�ַ�������
		return op.iStringTableIndex;
	case OP_TYPE_ABS_STACK_INDEX:					//���Զ�ջ����
		return getStackValue(op.iStackIndex);
	case OP_TYPE_REL_STACK_INDEX:					//��Զ�ջ����
		return getStackValue(getOpStackIndex(opIndex));
	case OP_TYPE_INSTR_INDEX:						//ָ������
		return op.iInstrIndex;
	case OP_TYPE_FUNC_INDEX:						//��������
		return op.iFuncIndex;
	case OP_TYPE_HOST_API_CALL_INDEX:				//�ں�api��������
		return op.iHostAPICallIndex;
	case OP_TYPE_REG:								//�Ĵ���
		return mReg[op.iReg];
	default:
		return 0;
	}
}

int Processor::getStackValue(int stackIndex) {
	int value = 0;
	if (stackIndex >= 0) {				//ȫ�ֱ���
		value = mMemory->readWord(mSS + stackIndex);
	}
	else if (stackIndex < 0) {			//�ֲ�����
		value = mMemory->readWord(mSS + (mBP + stackIndex)); 
	}
	return value;
}

void Processor::setStackValue(int stackIndex, int value) {
	if (stackIndex >= 0) {				//ȫ�ֱ���
		mMemory->writeWord(mSS + stackIndex, value);
	}
	else if (stackIndex < 0) {			//�ֲ�����
		mMemory->writeWord(mSS + (mBP + stackIndex), value);
	}
}

void Processor::handleIrq() {
	if (getFlag(0) == true) {						//�жϱ�����
		return;
	}
	if (mInt->hasIrq()) {							//���ж�����
		unsigned char intNum;						//�ж����ͺ�

		mInt->respondInt(intNum);					//��Ӧ�жϲ���ȡ�ж����ͺ�

		CPUDebug("handle irq, int num: ", 3);
		CPUDebugn(intNum, 3);

		mCore->callIntService(intNum);				//���ö�Ӧ�жϷ���
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
	unsigned char IR[32];					//��ǰָ��Ĵ���
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
		fetchIstr();					//ȡָ

		interprete();					//ָ�����벢ִ��

		handleIrq();					//�����ж�

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

	for (unsigned short index = 0; index < 4; ++index) {				//����Ĵ���ֵ
		push(mReg[index]);
	}
	push(mCS);															//�������λ�ַ
	push(mDS);															//�������ݶλ�ַ
	push(mFS);															//���溯�����ַ
	push(mAS);															//����ϵͳAPI���ñ��ַ
	push(mBP);															//���浱ǰ����֡��ַ

	push(mPC);															//���������
	curr->setStackTop(mSP);												//�����ջָ��
	curr->setStackBase(mSS);									
}

void Processor::reloadContext() {										
	OSPcb *curr = mCore->getPcbCurr();

	CPUDebug("load sp and ss.", 3);
	mSS = curr->getStackBase();											//��ȡջ��ַ
	mSP = curr->getStackTop();											//��ȡջ����ַ
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
