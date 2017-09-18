//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_FRAME_TABLE_ITEM_H
#define SDOS_FRAME_TABLE_ITEM_H

class FrameTableItem {
private:
    long         mFrameAddress;
    long         mLogicAddress;
    unsigned int mPid;
    bool         mOccupied;
public:
    explicit FrameTableItem(long frameAddress);
    bool operator == (const FrameTableItem&);
    void clear();
    bool occupy(unsigned int pid, long logicAddress);
};

#endif //SDOS_FRAME_TABLE_ITEM_H
