#include <vector>
#include "OSCore.h"
#include "OSPcbManager.h"
#include "OSSemManager.h"
#include "OSScheduler.h"
#include "fs.h"
#include "processor.h"
#include "mmu.h"
#include "printer.h"
#include "voicer.h"


OSCore *OSCore::gInstance = NULL;

void OSCore::initIdle() {
	OSDebugStrn("os create idle proc", 2);
	
	UINT8 err;
	OSPcb *pcb = mPcbManager->getFreePCB(err);								//获取空闲PCB

	UINT16 pid = pcb->getPid();

	//申请进程内存
	mMem->allocProcess(pid, "IDLE.FSE");

	err = loadExecFile("IDLE.FSE", pcb);									//载入程序

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
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("VOICE"), &OSCore::api_voice));
	mApiTable.insert(std::make_pair<std::string, API_FUNC>(std::string("PRINTER"), &OSCore::api_printer));

}

UINT8 OSCore::loadExecFile(const char *path, OSPcb * pcb, UINT32 base) {
	//FILE *pExecFile;
	FS *fileSys = FS::getInstance();
	UINT32 fileIndex = 0;
	
	ExecFileHeader header;													//可执行文件头
	UINT32 instrNum = 0;													//指令条数
	UINT32 stringNum = 0;													//字符串数量
	UINT32 funcNum = 0;														//函数数量
	UINT32 apiNum = 0;														//系统调用数量

	UINT32 codeSegSize = 0;													//代码段大小

	UINT32 codeSeg;															//代码段起始地址
	UINT32 stringSeg;														//字符串表起始地址
	UINT32 funcSeg;															//函数表起始地址
	UINT32 apiSeg;															//api表起始地址

	UINT16 pid = pcb->getPid();

	BYTE buffer[1024 * 512];												//读文件缓冲区

	if (!strstr(path, EXEC_FILE_EXT)) {										//检查可执行文件后缀
		return ERR_INVALID_FSE;
	}

	if (!fileSys->isExist(path)) {											//检查能否找到文件	
		return ERR_FILE_IO;
	}

	//if (!(pExecFile = fopen(path, "rb"))) {											
	//	return ERR_FILE_IO;
	//}

	//fread((void*)&header, sizeof(header), 1, pExecFile);					//读取头部
	fileSys->readFile(path, (void*)&header, sizeof(header), fileIndex);				
	fileIndex += sizeof(header);
	if (header.versionMajor != 0 || header.versionMinor != 1) {				//判断版本号
		return ERR_INVALID_VERSION;
	}
	if (header.stackSize == 0) {											//判断堆栈大小
		header.stackSize = DEF_STACK_SIZE;									
	}

	//fread((void*)&instrNum, sizeof(int), 1, pExecFile);						//读取指令数量
	fileSys->readFile(path, (void*)&instrNum, sizeof(int), fileIndex);
	fileIndex += sizeof(int);
	codeSegSize = instrNum * INSTR_LEN;
	codeSeg = base;															//设置基址寄存器
	//fread((void*)buffer, 1, instrNum * INSTR_LEN, pExecFile);				//读取代码段
	fileSys->readFile(path, (void*)buffer, instrNum * INSTR_LEN, fileIndex);
	fileIndex += instrNum * INSTR_LEN;
	//mMem->writeBlock(buffer, base, instrNum * INSTR_LEN);					//将代码段写入内存
	mMem->write(codeSeg, (void*)buffer, instrNum * INSTR_LEN, pid);

	unsigned int memIndex = codeSegSize;

	//fread((void*)&stringNum, sizeof(int), 1, pExecFile);					//读取字符串表表长	
	fileSys->readFile(path, (void*)&stringNum, sizeof(int), fileIndex);
	fileIndex += sizeof(int);
	stringSeg = base + memIndex;											//设置基址寄存器
	unsigned int stringLen;													//读取每一个字符串长度和字符串
	for (unsigned int index = 0; index < stringNum; ++index) {
		//fread((void*)&stringLen, sizeof(int), 1, pExecFile);				//读取字符串长度
		fileSys->readFile(path, (void*)&stringLen, sizeof(int), fileIndex);
		fileIndex += sizeof(int);
		//mMem->writeWord(base + memIndex, stringLen);
		mMem->write(base + memIndex, (void*)&stringLen, sizeof(int), pid);
		memIndex += sizeof(int);

		//fread((void*)buffer, 1, stringLen, pExecFile);						//读取字符串
		//mMem->writeBlock(buffer, base + memIndex, stringLen);
		fileSys->readFile(path, (void*)buffer, stringLen, fileIndex);
		fileIndex += stringLen;
		mMem->write(base + memIndex, (void*)buffer, stringLen, pid);
		memIndex += stringLen;
	}

	fileSys->readFile(path, (void*)&funcNum, sizeof(int), fileIndex);
	fileIndex += sizeof(int);
	//fread((void*)&funcNum, sizeof(int), 1, pExecFile);						//读取函数表表长
	funcSeg = base + memIndex;												//设置基址寄存器
	unsigned int funcTableSize = funcNum * sizeof(int) * 2;					//计算函数表大小
	//fread((void*)&buffer, 1, funcTableSize, pExecFile);						//读取函数表到内存
	fileSys->readFile(path, (void*)buffer, funcTableSize, fileIndex);
	fileIndex += funcTableSize;
	//mMem->writeBlock(buffer, base + memIndex, funcTableSize);
	mMem->write(funcSeg, (void*)buffer, funcTableSize, pid);
	memIndex += funcTableSize;

	//fread((void*)&apiNum, sizeof(int), 1, pExecFile);						//读取api调用表表长
	fileSys->readFile(path, (void*)&apiNum, sizeof(int), fileIndex);
	fileIndex += sizeof(int);
	apiSeg = base + memIndex;												//设置基址寄存器
	unsigned int apiLen;													//读取api调用表到内存
	for (unsigned int index = 0; index < apiNum; ++index) {
		//fread((void*)&apiLen, sizeof(int), 1, pExecFile);					//读取长度
		fileSys->readFile(path, (void*)&apiLen, sizeof(int), fileIndex);
		fileIndex += sizeof(int);
		//mMem->writeWord(base + memIndex, apiLen);
		mMem->write(base + memIndex, (void*)&apiLen, sizeof(int), pid);
		memIndex += sizeof(int);

		//fread((void*)&buffer, 1, apiLen, pExecFile);						//读取api名
		fileSys->readFile(path, (void*)buffer, apiLen, fileIndex);
		fileIndex += apiLen;
		//mMem->writeBlock(buffer, base + memIndex, apiLen);
		mMem->write(base + memIndex, (void*)buffer, apiLen, pid);
		memIndex += apiLen;
	}

	pcb->init(path, strlen(path), memIndex + base, header.stackSize, 0, PROC_STATE_READY);		

	//UINT32 ss = memIndex + base;
	//UINT32 sp = header.stackSize - 1;
	//UINT32 bp = sp - 3;			

	UINT32 ss = mMem->getProcess(pid)->getStackBaseAdderss() - header.stackSize + 1;
	UINT32 sp = header.stackSize - 1;
	UINT32 bp = sp - 3;

	//mMem->writeWord(ss + sp - 3, num);
	//mSP -= 4;

	//主函数堆栈
	//int mainFuncSize = mMem->readWord(funcSeg + header.mainFuncIndex * 8 + 4);	//获取主函数局部变量大小和入口点
	//int mainFuncEntry = mMem->readWord(funcSeg + header.mainFuncIndex * 8);
	int mainFuncSize = mMem->readHalfWord(pid, funcSeg + header.mainFuncIndex * 8 + 4);
	int mainFuncEntry = mMem->readHalfWord(pid, funcSeg + header.mainFuncIndex * 8);

	//mMem->writeWord(ss + sp - 3, 0);				//压入一个空值，保持操作一致
	int holder = 0;
	mMem->write(base + memIndex, (void*)&holder, sizeof(int), pid);
	sp -= 4;

	sp -= mainFuncSize;								//压入栈帧
	
	//mMem->writeWord(ss + sp - 3, mainFuncSize);	//压入栈帧大小
	mMem->write(ss + sp - 3, (void*)&mainFuncSize, sizeof(int), pid);
	sp -= 4;

	//压入各个寄存器初值
	for (int i = 0; i < 4; ++i) {
		//mMem->writeWord(ss + sp - 3, 0);			//通用寄存器
		mMem->write(ss + sp - 3, (void*)&holder, sizeof(int), pid);
		sp -= 4;
	}

	//mMem->writeWord(ss + sp - 3, codeSeg);			//压入代码段基址
	mMem->write(ss + sp - 3, (void*)&codeSeg, sizeof(int), pid);
	sp -= 4;

	//mMem->writeWord(ss + sp - 3, stringSeg);		//压入字符串段基址
	mMem->write(ss + sp - 3, (void*)&stringSeg, sizeof(int), pid);
	sp -= 4;

	//mMem->writeWord(ss + sp - 3, funcSeg);			//压入函数段基址
	mMem->write(ss + sp - 3, (void*)&funcSeg, sizeof(int), pid);
	sp -= 4;

	//mMem->writeWord(ss + sp - 3, apiSeg);			//压入api段基址
	mMem->write(ss + sp - 3, (void*)&apiSeg, sizeof(int), pid);
	sp -= 4;

	//mMem->writeWord(ss + sp - 3, bp);				//压入bp
	mMem->write(ss + sp - 3, (void*)&bp, sizeof(int), pid);
	sp -= 4;

	//mMem->writeWord(ss + sp - 3, mainFuncEntry * INSTR_LEN);	//压入主函数入口地址，即pc
	int pc = mainFuncEntry * INSTR_LEN;
	mMem->write(ss + sp - 3, (void*)&pc, sizeof(int), pid);
	sp -= 4;

	pcb->setStackBase(ss);				//设置堆栈基址和栈顶地址，堆栈段紧跟在api调用表后面
	pcb->setStackTop(sp);

	//fclose(pExecFile);

	return ERR_NO_ERR;
}

void OSCore::print(const std::string & text) {
	std::string consolePath = FS::getInstance()->getPath(".console");
	std::ofstream console(consolePath.c_str(), std::ios_base::app);

	if (!console.is_open()) {
		return;
	}

	console << getPcbCurr()->getPid() << ":" << text << std::endl;

	console.close();
}

void OSCore::getProcStates(std::vector<ProcState>& info) {
	mPcbManager->getProcStates(info);
}

F32 OSCore::getCpuUsage() {
	return mCPUUsage;
}

BOOLEAN OSCore::handleApiCall(const std::string & funcName) {
	APITABLE::iterator find = mApiTable.find(funcName);
	
	if (find == mApiTable.end()) {
		return FALSE;
	}

	OSDebugStrn(funcName.c_str(), 3);

	API_FUNC api = find->second;
	(this->*api)();															//调用对应的API函数
	return TRUE;
}

INT8 OSCore::processCreate(const char *path, UINT8 &err, UINT8 prio) {
	std::unique_lock<std::mutex> lck(mMtx);

	err = ERR_NO_ERR;
	OSPcb *pcb = mPcbManager->getFreePCB(err);								//获取空闲PCB

	if (err != ERR_NO_ERR) {												//获取失败
		return -1;
	}

	UINT16 pid = pcb->getPid();

	//申请进程内存。。。。。。。。。。。。测试是每个进程1M大小，对齐
	mMem->allocProcess(pid, path);

	err = loadExecFile(path, pcb);							//载入程序

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
	std::unique_lock<std::mutex> lck(mMtx);

	if (pid == PID_IDLE) {
		OSDebugStrn("os delete idle process", 2);
		return ERR_DEL_IDLE;
	}

	OSPcb *toDel = mScheduler->getCurrPcb();
	if (toDel->getPid() == pid || pid == PID_CURR) {		//如果删除的是当前正在执行的进程
		OSDebugStrn("os delete current process", 2);
		mScheduler->removeRunning();						//移除当前进程
		mScheduler->sched();								//进行调度，选择新的进程执行
		mPcbManager->returnPCB(toDel);						//返还pcb
		mCPU->reloadContext();								//上下文切换
		return ERR_NO_ERR;
	}

	if (!mPcbManager->isCreated(pid)) {						//如果要删除的任务未被创建
		OSDebugStrn("os delete invalid process", 2);
		return ERR_DEL_INVALID;
	}

	toDel = mPcbManager->getPCB(pid);						//如果删除其他进程，先获取其pcb
	if (toDel->getState() == PROC_STATE_READY) {			//如果是就绪状态，从就绪队列中删除
		mScheduler->removeFromReadyList(pid);
		mPcbManager->returnPCB(toDel);
		return ERR_NO_ERR;
	}
	else {													//如果是等待状态，判断是等待事件发生还是挂起
		if (toDel->isWaitingEvent()) {						//如果是等待信号量						
			toDel->removeFromSem();							//从等待的信号量的队列中删除该任务
		}
		mScheduler->removeFromWaitingList(pid);
		mPcbManager->returnPCB(toDel);
		return ERR_NO_ERR;
	}
}

UINT8 OSCore::processDelay(UINT16 delay) {
	std::unique_lock<std::mutex> lck(mMtx);

	OSPcb *toDelay = mScheduler->getCurrPcb();				//获取当前进程pcb
	toDelay->delay(delay);									//设置延时时间
	toDelay->setState(PROC_STATE_SUSPEND);					//设置状态为挂起

	mCPU->saveContext();									//保存上文
	mScheduler->removeRunning();							//将该进程加入等待队列，调度新进程
	mScheduler->addWaitingPcb(toDelay);
	mScheduler->sched();
	mCPU->reloadContext();									//切换下文

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
	INT32 stringIndex;						//字符串在表中的索引
	INT32 stringSize;
	INT32 stringStart;
	CHAR buffer[1024];						//字符串缓冲

	mCPU->pop(stringIndex);					//获取字符串索引

	//stringSize = mMem->readWord(mCPU->getDS() + stringIndex);
	stringSize = mMem->readHalfWord(mScheduler->getCurrPcb()->getPid(), mCPU->getDS() + stringIndex);
	
	//mMem->readBlock((BYTE*)buffer, mCPU->getDS() + stringIndex + 4, stringSize);
	stringStart = mCPU->getDS() + stringIndex + 4;
	for (UINT32 index = 0; index < stringSize; ++index) {
		buffer[index] = mMem->readByte(mScheduler->getCurrPcb()->getPid(), stringStart + index);
	}
	buffer[stringSize] = '\0';

	print(buffer);
}

void OSCore::api_process_create() {
	INT32 procNameIndex;
	INT32 nameSize;
	INT32 pid;
	UINT8 err;
	CHAR buffer[1024];
	

	mCPU->pop(procNameIndex);					//获取字符串索引

	//nameSize = mMem->readWord(mCPU->getDS() + procNameIndex);
	nameSize = mMem->readHalfWord(mScheduler->getCurrPcb()->getPid(), mCPU->getDS() + procNameIndex);

	//mMem->readBlock((BYTE*)buffer, mCPU->getDS() + procNameIndex + 4, nameSize);
	UINT32 nameStart = mCPU->getDS() + procNameIndex + 4;
	for (UINT32 index = 0; index < nameSize; ++index) {
		buffer[index] = mMem->readWord(mScheduler->getCurrPcb()->getPid(), nameStart + index);
	}
	buffer[nameSize] = '\0';

	pid = processCreate(buffer, err);
	mCPU->setReg(1, pid);						//返回值
}

void OSCore::api_process_delete() {
	INT32 pid;
	UINT8 err;
	mCPU->pop(pid);

	err = processDelete(pid);
	mCPU->setReg(1, err);						//返回值
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

void OSCore::api_voice() {
    INT32 procNameIndex;
    INT32 nameSize;
    CHAR buffer[1024];

    mCPU->pop(procNameIndex);					//获取字符串索引
    nameSize = mMem->readHalfWord(mScheduler->getCurrPcb()->getPid(), mCPU->getDS() + procNameIndex);
    UINT32 nameStart = mCPU->getDS() + procNameIndex + 4;
    for (UINT32 index = 0; index < nameSize; ++index) {
        buffer[index] = mMem->readWord(mScheduler->getCurrPcb()->getPid(), nameStart + index);
    }
    buffer[nameSize] = '\0';
    Voicer::speak(buffer);
}

void OSCore::api_printer() {
    INT32 procNameIndex;
    INT32 nameSize;
    CHAR buffer[1024];

    mCPU->pop(procNameIndex);					//获取字符串索引
    nameSize = mMem->readHalfWord(mScheduler->getCurrPcb()->getPid(), mCPU->getDS() + procNameIndex);
    UINT32 nameStart = mCPU->getDS() + procNameIndex + 4;
    for (UINT32 index = 0; index < nameSize; ++index) {
        buffer[index] = mMem->readWord(mScheduler->getCurrPcb()->getPid(), nameStart + index);
    }
    buffer[nameSize] = '\0';
    std::vector<std::string> result = Printer::PRINT(buffer);
    for (std::string line: result)
        print(line);
    mCPU->setReg(1, !result.empty());
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
		return TRUE;		//获取信号量成功
	}
	else {
		return FALSE;		//获取信号量失败
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
	if (sem->hasPend()) {					//如果还有任务在等待信号量，则不能删除
		return ERR_SEM_DEL_PENDING;
	}
	mSemManager->returnSem(id);				//通知信号量管理器标记信号量空闲
	return ERR_NO_ERR;
}

UINT8 OSCore::semPend(UINT16 id, UINT16 delay) {
	OSSem *sem = mSemManager->getSem(id);
	if (!sem) {
		return ERR_SEM_INVALID;
	}
	
	OSPcb *curr = mScheduler->getCurrPcb();
	if (sem->wait(curr, delay)) {					//不需要等待信号量
		return ERR_NO_ERR;
	}
													//任务需要等待信号量
	mCPU->saveContext();							//cpu保存任务内容	
	mScheduler->removeRunning();					//移除正在进行的进程
	mScheduler->addWaitingPcb(curr);				//进程添加到调度器等待队列中
	mScheduler->sched();							//调度新的进程
	mCPU->reloadContext();							//载入新的任务内容
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
	mCPU->saveContext();						//保存被中断的进程CPU内容	
	switch (intNum) {							//根据中断类型号执行中断服务程序
	case 0:
		timeTick();
	default:
		break;
	}
	mCPU->reloadContext();						//恢复被中断的进程CPU内容
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
	//mMem = Memory::getInstance();
	mMem = MMU::getInstance();

	mPcbManager = OSPcbManager::getInstance();			//初始化pcb模块
	mPcbManager->init();
	OSDebugStrn("os pcb modole finish init", 2);
	mScheduler = OSScheduler::getInstance();			//初始化调度模块
	mScheduler->init();
	OSDebugStrn("os schedule modole finish init", 2);	
	mSemManager = OSSemManager::getInstance();
	mSemManager->init();

	initIdle();								

//	initTestProc();

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
	mScheduler->sched();					//找到第一个就绪程序	
	OSDebugStrn("os find first to run", 2);
	OSPcb *toRun = mScheduler->getCurrPcb();

	mCPU->reloadContext();					//设置CPU内容
	OSDebugStrn("os start", 2);
	mCPU->exec();							//开始执行
}

void OSCore::timeTick() {
	if (mIsRunning == FALSE) {
		return;
	}

	mTime++;
	if (mTime % OS_TICKS_PER_SEC == 0) {				//计算空闲时间
		UINT32 hz = CPU_RATE_M;
		mCPUUsage = (F32)(hz - mFreeTime) / (F32)hz;
		clearFreeTime();
	}

	mScheduler->caculateDelay();						//计算各进程的延迟时间

	if (mScheduler->toSwapOutCurr()) {					//判断是否要调度
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

