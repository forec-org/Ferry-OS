#include "Memory.h"
#include <memory.h>

Memory *Memory::gInstance = NULL;

Memory::Memory() {
	mMemory = NULL;
	mMemorySize = 0;
}


Memory::~Memory() {
	delete [] mMemory;
	mMemory = NULL;
	mMemorySize = 0;
}

void Memory::init(UINT32 size) {
	mMemory = new BYTE[size];
	mMemorySize = size;
}

BYTE Memory::readByte(UINT32 address) {
	if (address >= mMemorySize) {
		return 0;
	}
	return mMemory[address];
}

UINT16 Memory::readHalfWord(UINT32 address) {
	if (address + 1 >= mMemorySize) {
		return 0;
	}
	UINT16 halfword = 0;
	halfword |= mMemory[address + 1];
	halfword <<= 8;
	halfword |= mMemory[address];
	return halfword;
}

UINT32 Memory::readWord(UINT32 address) {
	if (address + 3 >= mMemorySize) {
		return 0;
	}
	UINT32 word = 0;
	memcpy((void*)&word, (void*)&(mMemory[address]), sizeof(word));
	return word;
}

void Memory::readBlock(BYTE * block, UINT32 address, UINT32 size) {
	//memCopy(block, &mMemory[address], size);
	memcpy((void*)block, (void*)&(mMemory[address]), size);
}

void Memory::writeByte(UINT32 address, BYTE byte) {
	if (address >= mMemorySize) {
		return;
	}
	mMemory[address] = byte;
}

void Memory::writeHalfWord(UINT32 address, UINT16 halfWord) {
	if (address + 1 >= mMemorySize) {
		return;
	}
	mMemory[address] = (unsigned char)(halfWord & 0xff);
	mMemory[address + 1] = (unsigned char)((halfWord >> 8) & 0xff);
}

void Memory::writeWord(UINT32 address, UINT32 word) {
	if (address + 3 >= mMemorySize) {
		return;
	}
	memcpy((void*)&(mMemory[address]), (void*)&word, sizeof(word));
}

void Memory::writeBlock(BYTE * block, UINT32 address, UINT32 size) {
	memcpy((void*)&(mMemory[address]), (void*)block, size);
}

UINT8 Memory::getChar(UINT32 addr) {
	return mMemory[addr];
}

Memory * Memory::getInstance() {
	if (gInstance == NULL) {
		gInstance = new Memory();
	}
	return gInstance;
}

void Memory::release() {
	delete gInstance;
	gInstance = NULL;
}

