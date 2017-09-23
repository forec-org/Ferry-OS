#include "IntController.h"
#include "SystemConfig.h"

IntController *IntController::gInstance = nullptr;

unsigned char IntController::highestPrioMap[256] = {		//����һ��8λ����ֵΪ1�����λ���±�
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
	unsigned char highestReq = highestPrioMap[mIrr];	//��ȡ������������ȼ���ߵ��жϺ�
	
	if (mIsr == 0) {									//�����ǰû���ж����ڱ�����
		mIntNum = highestReq;							//����Ӧ���ȼ���ߵ��ж�
		return true;
	}

	unsigned char highestWork = highestPrioMap[mIsr];	//��ȡ���ڷ�������ȼ���ߵ��жϺ�

	if (highestReq < highestWork) {						//�������������ж����ȼ����ڵ�ǰ���ڷ�����ж����ȼ�
		mIntNum = highestReq;							//����Ӧ���ж�
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
	prioJudge();							//�ҳ����ȼ���ߵ��ж����ͺ�
	setIsr(mIntNum);						//��λ���ڷ���Ĵ���
	clearIntReq(mIntNum);					//�����ж�����Ĵ���
	if (mIrr == (unsigned char)0) {			//û�������ж�����
		mIrq = false;
	}
	intNum = mIntNum;
}

//void IntController::endInt() {
//	unsigned char index = highestPrioMap[mIsr];			//�ҵ����ڷ�������ȼ���ߵ��жϺ��±�
//	clearIsr(index);									//��շ���Ĵ�����Ӧλ
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
