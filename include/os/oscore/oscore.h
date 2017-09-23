#pragma once

#include "OS.h"
#include <string>
#include <unordered_map>
#include <mutex>
#include <vector>	

class OSPcb;
class OSPcbManager;
class OSScheduler;
class Processor;
class OSSemManager;
struct OSProcFile;
class MMU;
//class Memory;

class OSCore {
	static OSCore *gInstance;					

	typedef void (OSCore::*API_FUNC)(void);					//api函数指针
	typedef std::unordered_map<std::string, API_FUNC> APITABLE;

private:
	BOOLEAN			mIsRunning;								//内核是否启动
	UINT8			mIntNesting;							//中断嵌套层数，目前支持单级中断

	OSPcbManager	*mPcbManager;							//进程控制块管理模块
	OSScheduler		*mScheduler;							//调度模块
	OSSemManager	*mSemManager;							//信号量管理模块

	APITABLE		mApiTable;								//供用户程序调用的api表

	UINT32			mTime;									//系统时间（节拍值）
	UINT16			mFreeTime;								//1S中空闲的CPU周期数，可用于计算CPU利用率
	F32				mCPUUsage;								//cpu利用率

	Processor		*mCPU;									//CPU模拟
	//Memory			*mMem;									//内存模拟
	MMU				*mMem;	

	std::mutex		mMtx;									//互斥锁

private:
	void			initIdle();								//初始化空转程序
	
	void			initTestProc();							//初始化测试程序

	void			initApiTable();							//初始化api表					

	UINT8			loadExecFile(const char *path,			//装载可执行文件
								 OSPcb *pcb, 
								 UINT32 base = 0);				

	OSCore();

public:													
	BOOLEAN			handleApiCall(const std::string &funcName);		
															//处理进程的系统调用

	void			incFreeTime();							//空闲CPU周期数增加
	void			clearFreeTime();						//清空空闲CPU周期数

	void			callIntService(UINT8 intNum);			//调用中断服务程序

	//供用户程序调用的api，用堆栈传参，返回值存到1号寄存器

	void			api_print();							//参数为字符串在字符串表中的索引
	void			api_process_create();					//参数为进程路径字符串的表中索引
	void			api_process_delete();					//参数为进程id
	void			api_sem_accept();						//参数为信号量id
	void			api_sem_pend();							//参数为等待信号量id和等待时长
	void			api_sem_post();							//参数为信号量id
	void			api_sem_delete();						//参数为信号量id
	void			api_sem_create();						//参数为信号量初始计数值和最大计数值
	void			api_voice();							//发声，参数为要读出的字符串
	void			api_printer();							//打印艺术字，参数为打印的字符串

public:											
	
	~OSCore();
	
	void			init();									//初始化内核

	void			start();								//开始进行系统的多任务调度

	void			timeTick();								//时钟中断调用，统计系统时间，
															//判断是否应该进行轮转调度

	OSPcb			*getPcbCurr();							//获取当前pcb

	UINT16			getFreeTime();							//获取空闲CPU周期数

	void			print(const std::string &text);			//打印

	void			getProcStates(std::vector<ProcState> &info);		//前台调用，获取进程状态

	F32				getCpuUsage();							//获取CPU利用率

	INT8			processCreate(const char *path,			//进程创建
								  UINT8 &err, 
								  UINT8 prio = 0);		

	UINT8			processDelete(UINT16 pid);				//删除任务
	UINT8			processDelay(UINT16 delay);				//任务挂起（延时）
	UINT8			processQuit();							//进程退出	

	BOOLEAN			semAccept(UINT16 id);					//不阻塞地等待信号量
	INT16			semCreate(UINT16 ctr,					//创建信号量，返回信号量id
							  UINT16 lgst, 
							  UINT8 &err);
															
	UINT8			semDelete(UINT16 id);					//信号量删除
	UINT8			semPend(UINT16 id, UINT16 delay);		//等待信号量，delay为0表示永久等待
	UINT8			semPost(UINT16 id);						//释放信号量

	void			intEnter();								//告知操作系统进行中断服务程序,暂时可能用不到
	void			intExit();								//退出中断处理
	void			enterCritical();						//进入系统临界区，因为内核不在cpu上执行，因此暂时用不到
	void			exitCritical();							//离开系统临界区，同上

	static OSCore	*getInstance();		
	static void		release();
};

