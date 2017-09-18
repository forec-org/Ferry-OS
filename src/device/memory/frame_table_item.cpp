//
// Created by 王耀 on 2017/9/18.
//

#include "frame_table_item.h"

FrameTableItem::FrameTableItem(long frameAddress) {
    mFrameAddress = frameAddress;
    mOccupied = false;
}

bool FrameTableItem::operator==(const FrameTableItem & other) {
    return mFrameAddress == other.mFrameAddress;
}

bool FrameTableItem::occupy(unsigned int pid, long logicAddress) {
    mPid = pid;
    mLogicAddress = logicAddress;
    mOccupied = true;
}

void FrameTableItem::clear() {
    mOccupied = false;
}