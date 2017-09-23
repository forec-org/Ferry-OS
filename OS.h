#pragma once

//-------------------------------------------------------------------------
//
//	变量类型定义
//
//-------------------------------------------------------------------------
typedef bool				BOOLEAN;
typedef unsigned char		BYTE;
typedef char				CHAR;
typedef unsigned char		UINT8;
typedef char				INT8;
typedef unsigned short		UINT16;
typedef short				INT16;
typedef unsigned int		UINT32;
typedef int					INT32;
typedef unsigned long long	UINT64;
typedef long long			INT64;
typedef float				F32;
typedef double				F64;

typedef unsigned int		STK_SIZE;

#define NULL				nullptr
#define FALSE				0
#define TRUE				1


//-------------------------------------------------------------------------
//
//	常量定义
//
//-------------------------------------------------------------------------

//任务状态码
#define PROC_STATE_READY				0x00						//任务就绪，可以被调入执行
#define PROC_STATE_RUNNING				0x01						//任务运行中
#define PROC_STATE_WAITING				0x02						//任务等待事件或信号量
#define PROC_STATE_SUSPEND				0x04						//任务挂起
#define PROC_STATE_TERMINATED			0x08						//任务终止，退出内存
#define PROC_STATE_SWAPPED_READY		0x10						//任务被换出时就绪
#define PROC_STATE_SWAPPED_WAITING		0x20						//任务被换出时等待事件或信号量
#define PROC_STATE_SWAPPED_SUSPEND		0x40						//任务被换出时挂起

#define PROC_STATE_WAITING_ANY			PROC_STATE_WAITING | PROC_STATE_SWAPPED_WAITING					
#define PROC_STATE_SUSPEND_ANY			PROC_STATE_SUSPEND | PROC_STATE_SWAPPED_SUSPEND					


//错误码
#define ERR_NO_ERR						0x00						//无错误
#define ERR_NULL_PTR					0x01						//空指针
#define ERR_SIZE_ZERO					0x02						//字符串长度为0
#define ERR_STACK_OVERFLOW				0x03						
#define ERR_TOO_LARGE_SIZE				0x04						

#define ERR_NO_FREE_PCB					0x05						//没有空闲pcb

#define ERR_DEL_INVALID					0x06						//删除不存在的程序
#define ERR_DEL_IDLE					0x07						//删除空转程序

#define ERR_NO_FREE_SEM					0x08						//没有空闲信号量
#define ERR_SEM_INVALID					0x09						//信号量不存在
#define ERR_SEM_DEL_PENDING				0x0a						//删除的信号量仍有进程等待
#define ERR_SEM_POST_NO_PEND			0x0b						//信号量的等待队列为空

#define ERR_FILE_IO						0x0c						//未找到可执行文件
#define ERR_INVALID_FSE					0x0d						//可执行文件后缀名不是FSE
#define ERR_INVALID_VERSION				0x0e						//无效的可执行文件版本

//pid
#define PID_IDLE						0x0000u						//空转进程pid
#define PID_CURR						0xffffu						//当前任务pid

//信号量
#define SEM_NAME_SIZE					64							//

//-------------------------------------------------------------------------
//
//	调试函数声明
//
//-------------------------------------------------------------------------
#define DEBUG_LEVEL 0
void	OSDebugStr(const char *tip, int level);
void	OSDebugStrn(const char *tip, int level);
void	OSDebugInt(int num, int level);
void	OSDebugIntn(int num, int level);
void	OSDebugFloat(F32 num, int level);
void	OSDebugFloatn(F32 num, int level);

#define LOG 1
void	OSLogStr(const char *tip);

//-------------------------------------------------------------------------
//
//	结构体
//
//-------------------------------------------------------------------------
typedef struct _execFileHeader {
	char			idString[4];		//文件id字符串
	unsigned short	versionMajor;		//大版本号
	unsigned short	versionMinor;		//小版本号
	unsigned int	stackSize;			//堆栈大小
	unsigned int	globalDataSize;		//全局数据大小
	unsigned int	mainFuncIndex;		//主函数大小
}ExecFileHeader;