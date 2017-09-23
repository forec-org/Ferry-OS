#pragma once

#include "OS.h"

class Memory {
	static Memory *gInstance;

private:
	//BYTE *mMemory;										//һ�����ڴ�
	UINT32 mMemorySize;										//�ڴ��С

public:
	BYTE *mMemory;											//һ�����ڴ�

	Memory();
	~Memory();

	void init(UINT32 size);

	BYTE readByte(UINT32 address);							//��ȡһ���ֽ�

	UINT16 readHalfWord(UINT32 address);					//��ȡһ������

	UINT32 readWord(UINT32 address);						//��ȡһ����

	void readBlock(BYTE *block, UINT32 address, UINT32 size);				//��ȡһ����

	void writeByte(UINT32 address, BYTE byte);				//дһ���ֽ�

	void writeHalfWord(UINT32 address, UINT16 halfWord);	//дһ������

	void writeWord(UINT32 address, UINT32 word);			//дһ����	

	void writeBlock(BYTE *block, UINT32 address, UINT32 size);				//дһ����

	UINT8 getChar(UINT32 addr);


	static Memory *getInstance();

	static void release();
};

