#include "IntController.h"
#include "SystemConfig.h"

IntController *IntController::gInstance = nullptr;

unsigned char IntController::highestPrioMap[256] = {		//保存一个8位数的值为1的最低位的下标
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F                             */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F                             */
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF                             */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF                             */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF                             */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF                             */
};

bool IntController::prioJudge() {
	unsigned char highestReq = highestPrioMap[mIrr];	//获取发起请求的优先级最高的中断号
	
	if (mIsr == 0) {									//如果当前没有中断正在被服务
		mIntNum = highestReq;							//则响应优先级最高的中断
		return true;
	}

	unsigned char highestWork = highestPrioMap[mIsr];	//获取正在服务的优先级最高的中断号

	if (highestReq < highestWork) {						//如果发起请求的中断优先级高于当前正在服务的中断优先级
		mIntNum = highestReq;							//则响应该中断
		return true;
	}
	return false;
}

IntController::IntController() {

}


IntController::~IntController() {
}

void IntController::init() {
	mIrq = false;
	
	mIrr &= 0x00;
	mIsr &= 0x00;
}

void IntController::setIntReq(unsigned char index) {
	mIrr |= (0x01 << index);
	mIrq = true;
}

void IntController::clearIntReq(unsigned char index) {
	mIrr &= ~(0x01 << index);
}

void IntController::setIsr(unsigned short index) {
	mIsr |= (0x01 << index);
}

void IntController::clearIsr(unsigned char index) {
	mIsr &= ~(0x01 << index);
}

bool IntController::hasIrq() {
	return mIrq;
}

void IntController::respondInt(unsigned char &intNum) {
	prioJudge();							//找出优先级最高的中断类型号
	setIsr(mIntNum);						//置位正在服务寄存器
	clearIntReq(mIntNum);					//清零中断请求寄存器
	if (mIrr == (unsigned char)0) {			//没有其他中断请求
		mIrq = false;
	}
	intNum = mIntNum;
}

//void IntController::endInt() {
//	unsigned char index = highestPrioMap[mIsr];			//找到正在服务的优先级最高的中断号下标
//	clearIsr(index);									//清空服务寄存器对应位
//}

IntController * IntController::getInstance() {
	if (gInstance == nullptr) {
		gInstance = new IntController();
	}
	return gInstance;
}

void IntController::release() {
	delete gInstance;
	gInstance = nullptr;
}
