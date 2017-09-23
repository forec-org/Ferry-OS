#pragma once

#include "OS.h"
#include <list>

class OSPcb;

class OSSem {
private:
	UINT16				mId;							//每个信号量有一个标识
	UINT16				mCtr;							//信号量计数器
	UINT16				mLargestCtr;					//信号量计数器最大值

	CHAR				mName[SEM_NAME_SIZE];			//信号量名

	std::list<OSPcb*>	mWaitingList;					//等待信号量的进程

public:
	OSSem();
	~OSSem();

	void				init(UINT16 initCtr, 
							 UINT16 largestCtr, 
							 const char *name = NULL, 
							 UINT16 length = 0);

	bool				hasPend();

	BOOLEAN				wait(OSPcb *pcb,				
							 UINT16 delay);

	OSPcb				*signal();

	void				setId(UINT16 id);

	UINT16				getId();	

	void				setCtr(UINT16 ctr);

	UINT16				getCtr();

	void				setLargest(UINT16 largest);

	UINT16				getLargest();

	void				setName(const char *name, 
								UINT16 length);

	void				removeWaiting(UINT16 pid);		//从等待队列中移除对应的进程
};

