#pragma once

//	任务控制块最大数量
#define OS_MAX_PCB 64

//	信号量控制块最大数量
#define OS_MAX_SEM 64

//	调度频率
#define OS_TICKS_PER_SCHED 5

//	每秒时钟中断数
#define OS_TICKS_PER_SEC 2

//	可执行文件后缀
#define EXEC_FILE_EXT				".FSE"		

//默认堆栈大小
#define DEF_STACK_SIZE				1024