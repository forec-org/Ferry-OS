#pragma once

#include "OS.h"

class Memory {
	static Memory *gInstance;

private:
	//BYTE *mMemory;										//一整块内存
	UINT32 mMemorySize;										//内存大小

public:
	BYTE *mMemory;											//一整块内存

	Memory();
	~Memory();

	void init(UINT32 size);

	BYTE readByte(UINT32 address);							//读取一个字节

	UINT16 readHalfWord(UINT32 address);					//读取一个半字

	UINT32 readWord(UINT32 address);						//读取一个字

	void readBlock(BYTE *block, UINT32 address, UINT32 size);				//读取一个块

	void writeByte(UINT32 address, BYTE byte);				//写一个字节

	void writeHalfWord(UINT32 address, UINT16 halfWord);	//写一个半字

	void writeWord(UINT32 address, UINT32 word);			//写一个字	

	void writeBlock(BYTE *block, UINT32 address, UINT32 size);				//写一个块

	UINT8 getChar(UINT32 addr);


	static Memory *getInstance();

	static void release();
};

