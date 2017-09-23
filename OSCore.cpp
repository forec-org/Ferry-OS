#include "OSCore.h"
#include "OSPcbManager.h"
#include "OSSemManager.h"
#include "OSScheduler.h"

#include "Processor.h"
#include "Memory.h"
#include "ProcFile.h"


OSCore *OSCore::gInstance = NULL;

void OSCore::initIdle() {
	OSDebugStrn("os create idle proc", 2);
	
	UINT8 err;
	OSPcb *pcb = mPcbManager->getFreePCB(err);								//��ȡ����PCB

	UINT16 pid = pcb->getPid();

	//��������ڴ档����������������������������ÿ������1M��С������

	err = loadExecFile("idle.FSE", pcb, pid * 1024 * 1024);					//�������

	mScheduler->setIdlePcb(pcb);

	OSDebugStrn("os idle create finish\n", 2);
}

void OSCore::initTestProc() {
	UINT8 err;
	//processCreate("PROC_DELAYTEST.FSE", err);
	//processCreate("PROC_PRINTTEST.FSE", err);
	//processCreate("PROC_CREATEPROCTEST.FSE", err);
	processCreate("PROC_SEMPRODUCTOR.FSE", err);
	//processCreate("PROC_SEMCONSUMER.FSE", err);
}

void OSCore::initApiTable() {
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("INCFREETIME"), &OSCore::incFreeTime));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("PRINT"), &OSCore::api_print));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("PROCESSCREATE"), &OSCore::api_process_create));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("PROCESSDELETE"), &OSCore::api_process_delete));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("SEMACCEPT"), &OSCore::api_sem_accept));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("SEMCREATE"), &OSCore::api_sem_create));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("SEMDELETE"), &OSCore::api_sem_delete));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("SEMPEND"), &OSCore::api_sem_pend));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("SEMPOST"), &OSCore::api_sem_post));
}

UINT8 OSCore::loadExecFile(const char *path, OSPcb * pcb, UINT32 base) {
	FILE *pExecFile;
	ExecFileHeader header;													//��ִ���ļ�ͷ
	UINT32 instrNum = 0;													//ָ������
	UINT32 stringNum = 0;													//�ַ�������
	UINT32 funcNum = 0;														//��������
	UINT32 apiNum = 0;														//ϵͳ��������

	UINT32 codeSegSize = 0;													//����δ�С

	UINT32 codeSeg;															//�������ʼ��ַ
	UINT32 stringSeg;														//�ַ�������ʼ��ַ
	UINT32 funcSeg;															//��������ʼ��ַ
	UINT32 apiSeg;															//api����ʼ��ַ

	BYTE buffer[1024 * 512];												//���ļ�������

	if (!strstr(path, EXEC_FILE_EXT)) {										//����ִ���ļ���׺
		return ERR_INVALID_FSE;
	}

	if (!(pExecFile = fopen(path, "rb"))) {									//����ܷ��ҵ��ļ�			
		return ERR_FILE_IO;
	}

	fread((void*)&header, sizeof(header), 1, pExecFile);					//��ȡͷ��
	if (header.versionMajor != 0 || header.versionMinor != 1) {				//�жϰ汾��
		return ERR_INVALID_VERSION;
	}
	if (header.stackSize == 0) {											//�ж϶�ջ��С
		header.stackSize = DEF_STACK_SIZE;									
	}

	fread((void*)&instrNum, sizeof(int), 1, pExecFile);						//��ȡָ������
	codeSegSize = instrNum * INSTR_LEN;
	codeSeg = base;															//���û�ַ�Ĵ���
	fread((void*)buffer, 1, instrNum * INSTR_LEN, pExecFile);				//��ȡ�����
	mMem->writeBlock(buffer, base, instrNum * INSTR_LEN);				//�������д���ڴ�

	unsigned int memIndex = codeSegSize;

	fread((void*)&stringNum, sizeof(int), 1, pExecFile);					//��ȡ�ַ������										
	stringSeg = base + memIndex;											//���û�ַ�Ĵ���
	unsigned int stringLen;													//��ȡÿһ���ַ������Ⱥ��ַ���
	for (unsigned int index = 0; index < stringNum; ++index) {
		fread((void*)&stringLen, sizeof(int), 1, pExecFile);				//��ȡ�ַ�������
		mMem->writeWord(base + memIndex, stringLen);
		memIndex += sizeof(int);

		fread((void*)buffer, 1, stringLen, pExecFile);						//��ȡ�ַ���
		mMem->writeBlock(buffer, base + memIndex, stringLen);
		memIndex += stringLen;
	}

	fread((void*)&funcNum, sizeof(int), 1, pExecFile);						//��ȡ�������
	funcSeg = base + memIndex;												//���û�ַ�Ĵ���
	unsigned int funcTableSize = funcNum * sizeof(int) * 2;					//���㺯�����С
	fread((void*)&buffer, 1, funcTableSize, pExecFile);						//��ȡ�������ڴ�
	mMem->writeBlock(buffer, base + memIndex, funcTableSize);
	memIndex += funcTableSize;

	fread((void*)&apiNum, sizeof(int), 1, pExecFile);						//��ȡapi���ñ��
	apiSeg = base + memIndex;												//���û�ַ�Ĵ���
	unsigned int apiLen;													//��ȡapi���ñ��ڴ�
	for (unsigned int index = 0; index < apiNum; ++index) {
		fread((void*)&apiLen, sizeof(int), 1, pExecFile);					//��ȡ����
		mMem->writeWord(base + memIndex, apiLen);
		memIndex += sizeof(int);

		fread((void*)&buffer, 1, apiLen, pExecFile);						//��ȡapi��
		mMem->writeBlock(buffer, base + memIndex, apiLen);
		memIndex += apiLen;
	}

	pcb->init(memIndex + base, header.stackSize, 0, PROC_STATE_READY);		

	UINT32 ss = memIndex + base;
	UINT32 sp = header.stackSize - 1;
	UINT32 bp = sp - 3;			

	//mMem->writeWord(ss + sp - 3, num);
	//mSP -= 4;

	//��������ջ
	int mainFuncSize = mMem->readWord(funcSeg + header.mainFuncIndex * 8 + 4);	//��ȡ�������ֲ�������С����ڵ�
	int mainFuncEntry = mMem->readWord(funcSeg + header.mainFuncIndex * 8);
		
	mMem->writeWord(ss + sp - 3, 0);				//ѹ��һ����ֵ�����ֲ���һ��
	sp -= 4;

	sp -= mainFuncSize;								//ѹ��ջ֡
	
	mMem->writeWord(ss + sp - 3, mainFuncSize);		//ѹ��ջ֡��С
	sp -= 4;

	//ѹ������Ĵ�����ֵ
	for (int i = 0; i < 4; ++i) {
		mMem->writeWord(ss + sp - 3, 0);			//ͨ�üĴ���
		sp -= 4;
	}

	mMem->writeWord(ss + sp - 3, codeSeg);			//ѹ�����λ�ַ
	sp -= 4;

	mMem->writeWord(ss + sp - 3, stringSeg);		//ѹ���ַ����λ�ַ
	sp -= 4;

	mMem->writeWord(ss + sp - 3, funcSeg);			//ѹ�뺯���λ�ַ
	sp -= 4;

	mMem->writeWord(ss + sp - 3, apiSeg);			//ѹ��api�λ�ַ
	sp -= 4;

	mMem->writeWord(ss + sp - 3, bp);				//ѹ��bp
	sp -= 4;

	mMem->writeWord(ss + sp - 3, mainFuncEntry * INSTR_LEN);	//ѹ����������ڵ�ַ����pc
	sp -= 4;

	pcb->setStackBase(memIndex + base);				//���ö�ջ��ַ��ջ����ַ����ջ�ν�����api���ñ����
	pcb->setStackTop(sp);

	fclose(pExecFile);

	return ERR_NO_ERR;
}

void OSCore::print(const std::string & text) {
	printf(text.c_str());
	printf("\n");
}

BOOLEAN OSCore::handleApiCall(const std::string & funcName) {
	APITABLE::iterator find = mApiTable.find(funcName);
	
	if (find == mApiTable.end()) {
		return FALSE;
	}

	OSDebugStrn(funcName.c_str(), 3);

	API_FUNC api = find->second;
	(this->*api)();															//���ö�Ӧ��API����
	return TRUE;
}

INT8 OSCore::processCreate(const char *path, UINT8 &err, UINT8 prio) {
	err = ERR_NO_ERR;
	OSPcb *pcb = mPcbManager->getFreePCB(err);								//��ȡ����PCB

	if (err != ERR_NO_ERR) {												//��ȡʧ��
		return -1;
	}

	UINT16 pid = pcb->getPid();

	//��������ڴ档����������������������������ÿ������1M��С������

	err = loadExecFile(path, pcb, pid * 1024 * 1024);						//�������

	if (err != ERR_NO_ERR) {
		mPcbManager->returnPCB(pcb);
		return -1;
	}
	mScheduler->addReadyPcb(pcb);
	return pcb->getPid();
}

UINT8 OSCore::processQuit() {
	return processDelete(PID_CURR);
}

UINT8 OSCore::processDelete(UINT16 pid) {
	if (pid == PID_IDLE) {
		OSDebugStrn("os delete idle process", 2);
		return ERR_DEL_IDLE;
	}

	OSPcb *toDel = mScheduler->getCurrPcb();
	if (toDel->getPid() == pid || pid == PID_CURR) {		//���ɾ�����ǵ�ǰ����ִ�еĽ���
		OSDebugStrn("os delete current process", 2);
		mScheduler->removeRunning();						//�Ƴ���ǰ����
		mScheduler->sched();								//���е��ȣ�ѡ���µĽ���ִ��
		mPcbManager->returnPCB(toDel);						//����pcb
		mCPU->reloadContext();								//�������л�
		return ERR_NO_ERR;
	}

	if (!mPcbManager->isCreated(pid)) {						//���Ҫɾ��������δ������
		OSDebugStrn("os delete invalid process", 2);
		return ERR_DEL_INVALID;
	}

	toDel = mPcbManager->getPCB(pid);						//���ɾ���������̣��Ȼ�ȡ��pcb
	if (toDel->getState() == PROC_STATE_READY) {			//����Ǿ���״̬���Ӿ���������ɾ��
		mScheduler->removeFromReadyList(pid);
		mPcbManager->returnPCB(toDel);
		return ERR_NO_ERR;
	}
	else {													//����ǵȴ�״̬���ж��ǵȴ��¼��������ǹ���
		if (toDel->isWaitingEvent()) {						//����ǵȴ��ź���						
			toDel->removeFromSem();							//�ӵȴ����ź����Ķ�����ɾ��������
		}
		mScheduler->removeFromWaitingList(pid);
		mPcbManager->returnPCB(toDel);
		return ERR_NO_ERR;
	}
}

UINT8 OSCore::processDelay(UINT16 delay) {
	OSPcb *toDelay = mScheduler->getCurrPcb();				//��ȡ��ǰ����pcb
	toDelay->delay(delay);									//������ʱʱ��
	toDelay->setState(PROC_STATE_SUSPEND);					//����״̬Ϊ����

	mCPU->saveContext();									//��������
	mScheduler->removeRunning();							//���ý��̼���ȴ����У������½���
	mScheduler->addWaitingPcb(toDelay);
	mScheduler->sched();
	mCPU->reloadContext();									//�л�����

	return ERR_NO_ERR;
}

void OSCore::incFreeTime() {
	mFreeTime++;
	OSDebugStr("free time: ", 3);
	OSDebugIntn(mFreeTime, 3);
}

void OSCore::clearFreeTime() {
	mFreeTime = 0;
}

void OSCore::api_print() {
	INT32 stringIndex;						//�ַ����ڱ��е�����
	INT32 stringSize;
	CHAR buffer[1024];						//�ַ�������

	mCPU->pop(stringIndex);					//��ȡ�ַ�������

	stringSize = mMem->readWord(mCPU->getDS() + stringIndex);
	mMem->readBlock((BYTE*)buffer, mCPU->getDS() + stringIndex + 4, stringSize);
	buffer[stringSize] = '\0';

	print(buffer);
}

void OSCore::api_process_create() {
	INT32 procNameIndex;
	INT32 nameSize;
	INT32 pid;
	UINT8 err;
	CHAR buffer[1024];
	

	mCPU->pop(procNameIndex);					//��ȡ�ַ�������

	nameSize = mMem->readWord(mCPU->getDS() + procNameIndex);
	mMem->readBlock((BYTE*)buffer, mCPU->getDS() + procNameIndex + 4, nameSize);
	buffer[nameSize] = '\0';

	pid = processCreate(buffer, err);
	mCPU->setReg(1, pid);						//����ֵ
}

void OSCore::api_process_delete() {
	INT32 pid;
	UINT8 err;
	mCPU->pop(pid);

	err = processDelete(pid);
	mCPU->setReg(1, err);						//����ֵ
}

void OSCore::api_sem_accept() {					
	INT32 semid;
	INT8 res;

	mCPU->pop(semid);

	res = semAccept(semid);

	mCPU->setReg(1, res);
}

void OSCore::api_sem_pend() {
	INT32 semid, delay;

	mCPU->pop(semid);
	mCPU->pop(delay);

	semPend(semid, delay);
}

void OSCore::api_sem_post() {
	INT32 semid;
	UINT8 err;

	mCPU->pop(semid);

	err = semPost(semid);
	mCPU->setReg(1, err);
}

void OSCore::api_sem_delete() {
	INT32 semid;
	UINT8 err;

	mCPU->pop(semid);

	err = semDelete(semid);
	mCPU->setReg(1, err);
}

void OSCore::api_sem_create() {
	INT32 initCtr, largestCtr, semId;
	UINT8 err;

	mCPU->pop(initCtr);
	mCPU->pop(largestCtr);

	semId = semCreate(initCtr, largestCtr, err);
	mCPU->setReg(1, semId);
}

UINT16 OSCore::getFreeTime() {
	return mFreeTime;
}

BOOLEAN OSCore::semAccept(UINT16 id) {
	OSSem *sem = mSemManager->getSem(id);
	if (!sem) {
		return FALSE;
	}

	UINT16 ctr = sem->getCtr();
	if (ctr > 0) {
		sem->setCtr(--ctr);
		return TRUE;		//��ȡ�ź����ɹ�
	}
	else {
		return FALSE;		//��ȡ�ź���ʧ��
	}	
}

INT16 OSCore::semCreate(UINT16 ctr, UINT16 lgst, UINT8 & err) {
	OSSem *sem = mSemManager->getFreeSem(err);
	if (err == ERR_NO_FREE_SEM) {		
		return -1;
	}
	sem->init(ctr, lgst);
	return sem->getId();
}

UINT8 OSCore::semDelete(UINT16 id) {
	OSSem *sem = mSemManager->getSem(id);
	if (!sem) {
		return ERR_SEM_INVALID;
	}
	if (sem->hasPend()) {					//������������ڵȴ��ź���������ɾ��
		return ERR_SEM_DEL_PENDING;
	}
	mSemManager->returnSem(id);				//֪ͨ�ź�������������ź�������
	return ERR_NO_ERR;
}

UINT8 OSCore::semPend(UINT16 id, UINT16 delay) {
	OSSem *sem = mSemManager->getSem(id);
	if (!sem) {
		return ERR_SEM_INVALID;
	}
	
	OSPcb *curr = mScheduler->getCurrPcb();
	if (sem->wait(curr, delay)) {					//����Ҫ�ȴ��ź���
		return ERR_NO_ERR;
	}
													//������Ҫ�ȴ��ź���
	mCPU->saveContext();							//cpu������������	
	mScheduler->removeRunning();					//�Ƴ����ڽ��еĽ���
	mScheduler->addWaitingPcb(curr);				//������ӵ��������ȴ�������
	mScheduler->sched();							//�����µĽ���
	mCPU->reloadContext();							//�����µ���������
	return ERR_NO_ERR;
}

UINT8 OSCore::semPost(UINT16 id) {
	OSSem *sem = mSemManager->getSem(id);
	if (!sem) {
		return ERR_SEM_INVALID;
	}

	OSPcb *toReady = sem->signal();
	if (toReady) {
		mScheduler->removeFromWaitingList(toReady->getPid());
		mScheduler->addReadyPcb(toReady);
		return ERR_NO_ERR;
	}
	return ERR_SEM_POST_NO_PEND;
}

void OSCore::callIntService(UINT8 intNum) {
	mCPU->saveContext();						//���汻�жϵĽ���CPU����	
	switch (intNum) {							//�����ж����ͺ�ִ���жϷ������
	case 0:
		timeTick();
	default:
		break;
	}
	mCPU->reloadContext();						//�ָ����жϵĽ���CPU����
}

OSCore::OSCore() {
	mIsRunning		= FALSE;
	mIntNesting		= 0;
	mPcbManager		= NULL;
	mScheduler		= NULL;
	mTime			= 0;
	mFreeTime		= 0;
}

OSCore::~OSCore() {
	OSPcbManager::release();
	OSScheduler::release();
}

void OSCore::init() {
	OSDebugStrn("os start init", 2);
	mCPU = Processor::getInstance();
	mMem = Memory::getInstance();

	mPcbManager = OSPcbManager::getInstance();			//��ʼ��pcbģ��
	mPcbManager->init();
	OSDebugStrn("os pcb modole finish init", 2);
	mScheduler = OSScheduler::getInstance();			//��ʼ������ģ��
	mScheduler->init();
	OSDebugStrn("os schedule modole finish init", 2);	
	mSemManager = OSSemManager::getInstance();
	mSemManager->init();

	initIdle();								

	initTestProc();

	initApiTable();
}

void OSCore::intEnter() {
	if (mIsRunning != TRUE) {
		return;
	}
	if (mIntNesting < 0xffu) {
		mIntNesting++;
	}
}

void OSCore::intExit() {
	if (mIsRunning != TRUE) {
		return;
	}
	if (mIntNesting > 0) {
		mIntNesting--;
	}
}

void OSCore::start() {
	if (mIsRunning == TRUE) {
		return;
	}
	mIsRunning = TRUE;

	OSDebugStrn("os sched new", 2);
	mScheduler->sched();					//�ҵ���һ����������	
	OSDebugStrn("os find first to run", 2);
	OSPcb *toRun = mScheduler->getCurrPcb();

	mCPU->reloadContext();					//����CPU����
	OSDebugStrn("os start", 2);
	mCPU->exec();							//��ʼִ��
}

void OSCore::timeTick() {
	if (mIsRunning == FALSE) {
		return;
	}

	mTime++;
	if (mTime % OS_TICKS_PER_SEC == 0) {				//�������ʱ��
		UINT32 hz = CPU_RATE_M;
		mCPUUsage = (F32)(hz - mFreeTime) / (F32)hz;
		print("cpu usage: ");
		printf("%f\n", mCPUUsage);
		clearFreeTime();
	}

	mScheduler->caculateDelay();						//��������̵��ӳ�ʱ��

	if (mScheduler->toSwapOutCurr()) {					//�ж��Ƿ�Ҫ����
		mScheduler->sched();
	}
}

OSPcb * OSCore::getPcbCurr() {
	return mScheduler->getCurrPcb();
}

void OSCore::enterCritical() {
}

void OSCore::exitCritical() {
}

OSCore * OSCore::getInstance() {
	if (gInstance == NULL) {
		gInstance = new OSCore();
	}
	return gInstance;
}

void OSCore::release() {
	delete gInstance;
	gInstance = NULL;
}

