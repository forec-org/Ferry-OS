//
// Created by 王耀 on 2017/9/18.
//

#include "frame_table_item.h"

FrameTableItem::FrameTableItem(unsigned long frameAddress) {
    mFrameAddress = frameAddress;
    mOccupied = false;
}

bool FrameTableItem::operator==(const FrameTableItem & other) {
    return mFrameAddress == other.mFrameAddress;
}

bool FrameTableItem::occupy(unsigned int pid, unsigned long logicalPage) {
    mPid = pid;
    mLogicPage = logicalPage;
    mOccupied = true;
    return true;
}

void FrameTableItem::clear() {
    mOccupied = false;
}

void FrameTableItem::updateFrame(unsigned long frameAddress, unsigned long logicalPage, unsigned int pid) {
    mFrameAddress = frameAddress;
    mLogicPage = logicalPage;
    mPid = pid;
    mOccupied = true;
}

bool FrameTableItem::isOccupiedBy(unsigned int pid, unsigned long logicalPage) {
    return mPid == pid && logicalPage == mLogicPage && mOccupied;
}

void FrameTableItem::setFrameAddress(unsigned long frameAddress) {
    mFrameAddress = frameAddress;
}